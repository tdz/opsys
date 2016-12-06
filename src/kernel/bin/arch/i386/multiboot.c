/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009-2010  Thomas Zimmermann
 *  Copyright (C) 2016       Thomas Zimmermann
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* required by multiboot.h */
#define grub_uint16_t   multiboot_uint16_t
#define grub_uint32_t   multiboot_uint32_t
#define GRUB_PACKED     __attribute__((packed))

#include <errno.h>
#include <multiboot.h>
#include <stddef.h>
#include "main.h"
#include "page.h"
#include "pageframe.h"
#include "pmem.h"

/*
 * Mmap-entry iteration
 */

const struct multiboot_mmap_entry*
first_mmap_entry(const struct multiboot_info* info)
{
    if (!(info->flags & MULTIBOOT_INFO_MEM_MAP)) {
        return NULL;
    }
    return (const struct multiboot_mmap_entry*)((uintptr_t)info->mmap_addr);
}

const struct multiboot_mmap_entry*
next_mmap_entry(const struct multiboot_info* info,
                const struct multiboot_mmap_entry* entry)
{
    uintptr_t next = ((uintptr_t)entry) + sizeof(entry->size) + entry->size;

    if (next >= info->mmap_addr + info->mmap_length) {
        return NULL;
    }
    return (const struct multiboot_mmap_entry*)next;
}

const struct multiboot_mmap_entry*
next_mmap_entry_of_type(const struct multiboot_info* info,
                        const struct multiboot_mmap_entry* entry,
                        multiboot_uint32_t type)
{
    do {
        entry = next_mmap_entry(info, entry);
    } while (entry && entry->type != type);

    return entry; // either of correct type or NULL
}

const struct multiboot_mmap_entry*
first_mmap_entry_of_type(const struct multiboot_info* info,
                         multiboot_uint32_t type)
{
    const struct multiboot_mmap_entry* entry = first_mmap_entry(info);

    if (entry && entry->type == type) {
        return entry;
    }
    return next_mmap_entry_of_type(info, entry, type);
}

/*
 * Module iteration
 */

const struct multiboot_mod_list*
first_mod_list(const struct multiboot_info* info)
{
    if (!(info->flags != MULTIBOOT_INFO_MODS)) {
        return NULL;
    }
    if (!info->mods_count) {
        return NULL;
    }
    return (const struct multiboot_mod_list*)(uintptr_t)info->mods_addr;
}

const struct multiboot_mod_list*
next_mod_list(const struct multiboot_info* info,
              const struct multiboot_mod_list* mod)
{
    const struct multiboot_mod_list* first =
        (const struct multiboot_mod_list*)(uintptr_t)info->mods_addr;

    ++mod;

    if (mod - first >= info->mods_count) {
        return NULL;
    }
    return mod;
}

static int
range_order(unsigned long beg1, unsigned long end1,
            unsigned long beg2, unsigned long end2)
{
    if (end1 <= beg2) {
        return -1;
    } else if (end2 <= beg1) {
        return 1;
    }
    return 0;
}

static unsigned long
find_unused_area(const struct multiboot_header *mb_header,
                 const struct multiboot_info *mb_info, unsigned long nframes)
{
    os_index_t kernel_pfindex =
        pageframe_index((void*)mb_header->load_addr);

    size_t kernel_nframes =
        pageframe_span((void*)mb_header->load_addr,
                       mb_header->bss_end_addr - mb_header->load_addr + 1);

    unsigned long pfoffset = 0;

    for (const struct multiboot_mmap_entry* mmap =
                first_mmap_entry_of_type(mb_info, MULTIBOOT_MEMORY_AVAILABLE);
            mmap && !pfoffset;
            mmap = next_mmap_entry_of_type(mb_info, mmap, MULTIBOOT_MEMORY_AVAILABLE)) {

        /* area page index and length */

        os_index_t area_pfindex = pageframe_index((void*)(uintptr_t)mmap->addr);
        size_t area_nframes = pageframe_count(mmap->len);

        /* area at address 0, or too small */
        if (!area_pfindex || (area_nframes < nframes)) {
            continue;
        }

        /* possible index */

        os_index_t pfindex = area_pfindex;

        /* check for intersection with kernel */

        if (!range_order(kernel_pfindex, kernel_pfindex + kernel_nframes,
                         pfindex, pfindex + nframes)) {
            pfindex = kernel_pfindex + kernel_nframes;
        }

        /* check for intersection with modules */

        while (!pfoffset &&
               ((pfindex + nframes) < (area_pfindex + area_nframes))) {

            for (const struct multiboot_mod_list* mod = first_mod_list(mb_info);
                    mod;
                    mod = next_mod_list(mb_info, mod)) {

                os_index_t mod_pfindex =
                    pageframe_index((void *)mod->mod_start);

                size_t mod_nframes =
                    pageframe_count(mod->mod_end - mod->mod_start);

                /* check intersection */

                if (range_order(mod_pfindex,
                                mod_pfindex + mod_nframes,
                                pfindex, pfindex + nframes)) {
                    /* no intersection, offset found */
                    pfoffset = pageframe_offset(pfindex);
                } else {
                    pfindex = mod_pfindex + mod_nframes;
                }
            }
        }
    }

    return pfoffset;
}

static size_t
available_mem(const struct multiboot_info* info)
{
    size_t mem = 0;

    for (const struct multiboot_mmap_entry* mmap = first_mmap_entry(info);
            mmap;
            mmap = next_mmap_entry(info, mmap)) {

        size_t end = mmap->addr + mmap->len;

        if (end > mem) {
            mem = end;
        }
    }

    return mem;
}

static int
multiboot_init_pmem(const struct multiboot_header *mb_header,
                       const struct multiboot_info *mb_info)
{
    size_t mem = available_mem(mb_info);
    if (!mem) {
        return -ENOMEM;
    }

    unsigned long pfcount = pageframe_span(0, mem);

    unsigned long pfindex = find_unused_area(mb_header,
                                             mb_info,
                                             pfcount >> PAGEFRAME_SHIFT);
    if (!pfindex) {
        return -ENOMEM;
    }

    return pmem_init(pfindex, pfcount);
}

static int
multiboot_init_pmem_kernel(const struct multiboot_header *mb_header)
{
        return pmem_set_flags(pageframe_index
                              ((void *)mb_header->load_addr),
                              pageframe_count(mb_header->bss_end_addr -
                                              mb_header->load_addr),
                              PMEM_FLAG_RESERVED);
}

static int
multiboot_init_pmem_mmap(const struct multiboot_info *mb_info)
{
    for (const struct multiboot_mmap_entry* mmap = first_mmap_entry(mb_info);
            mmap;
            mmap = next_mmap_entry(mb_info, mmap)) {

        os_index_t pfindex = pageframe_index((void*)(uintptr_t)mmap->addr);
        size_t nframes = pageframe_count(mmap->len);

        unsigned char flags = mmap->type == MULTIBOOT_MEMORY_AVAILABLE ?
                                PMEM_FLAG_USEABLE :
                                PMEM_FLAG_RESERVED;

        pmem_set_flags(pfindex, nframes, flags);
    }

    return 0;
}

static int
multiboot_init_pmem_module(const struct multiboot_mod_list* mod)
{
        return pmem_set_flags(pageframe_index((void *)mod->mod_start),
                              pageframe_count(mod->mod_end -
                                              mod->mod_start),
                              PMEM_FLAG_RESERVED);
}

static int
multiboot_init_pmem_modules(const struct multiboot_info *mb_info)
{
    int res = 0;

    for (const struct multiboot_mod_list* mod = first_mod_list(mb_info);
            mod;
            mod = next_mod_list(mb_info, mod)) {
        res = multiboot_init_pmem_module(mod);
        if (res < 0) {
            break;
        }
    }

    return res;
}

static int
multiboot_load_modules(struct task *parent,
                       const struct multiboot_info *mb_info)
{
    for (const struct multiboot_mod_list* mod = first_mod_list(mb_info);
            mod;
            mod = next_mod_list(mb_info, mod)) {
        /* ignore errors */
        execute_module(parent,
                       mod->mod_start,
                       mod->mod_end - mod->mod_start + 1,
                       (const char*)mod->cmdline);
    }

    return 0;
}

/**
 * The function |multiboot_init| is called from multiboot.S as the entry
 * point into the C code. Don't declare it 'static.'
 */
void
multiboot_init(const struct multiboot_header *mb_header,
               const struct multiboot_info *mb_info, void *stack)
{
    const static size_t NPAGES = (4 * 1024 * 1024) / PAGE_SIZE;

    /* init physical memory with lowest 4 MiB reserved for DMA,
     * kernel, etc */

    int res = multiboot_init_pmem(mb_header, mb_info);
    if (res < 0) {
        return;
    }

    pmem_set_flags(0, NPAGES, PMEM_FLAG_RESERVED);

    res = multiboot_init_pmem_kernel(mb_header);
    if (res < 0) {
        return;
    }

    res = multiboot_init_pmem_mmap(mb_info);
    if (res < 0) {
        return;
    }

    res = multiboot_init_pmem_modules(mb_info);
    if (res < 0) {
        return;
    }

    /* setup virtual memory and system services */

    struct task* task;
    res = general_init(&task, &stack);
    if (res < 0) {
        return;
    }

    /* load modules as ELF binaries */

    res = multiboot_load_modules(task, mb_info);
    if (res < 0) {
        return;
    }
}
