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
#include "main.h"
#include "page.h"
#include "pageframe.h"
#include "pmem.h"

static int
range_order(unsigned long beg1, unsigned long end1,
            unsigned long beg2, unsigned long end2)
{
        if (end1 <= beg2)
        {
                return -1;
        }
        else if (end2 <= beg1)
        {
                return 1;
        }

        return 0;
}

static unsigned long
find_unused_area(const struct multiboot_header *mb_header,
                 const struct multiboot_info *mb_info, unsigned long nframes)
{
        size_t i;
        unsigned long mmap_addr;
        unsigned long kernel_pfindex, kernel_nframes;
        unsigned long pfoffset;

        kernel_pfindex = pageframe_index((void *)mb_header->load_addr);
        kernel_nframes = pageframe_span((void *)mb_header->load_addr,
                                        mb_header->bss_end_addr -
                                        mb_header->load_addr + 1);

        mmap_addr = mb_info->mmap_addr;

        for (pfoffset = 0, i = 0; !pfoffset && (i < mb_info->mmap_length);)
        {
                const struct multiboot_mmap_entry *mmap;
                unsigned long area_pfindex;
                unsigned long area_nframes;
                unsigned long pfindex;

                mmap = (const struct multiboot_mmap_entry*)mmap_addr;

                /* next entry address  */

                mmap_addr += mmap->size + 4;
                i += mmap->size + 4;

                /* area is not useable */
                if (mmap->type != MULTIBOOT_MEMORY_AVAILABLE)
                {
                        continue;
                }

                /* area page index and length */

                area_pfindex = pageframe_index((void*)(uintptr_t)mmap->addr);
                area_nframes = pageframe_count(mmap->len);

                /* area at address 0, or too small */

                if (!area_pfindex || (area_nframes < nframes))
                {
                        continue;
                }

                /* possible index */

                pfindex = area_pfindex;

                /* check for intersection with kernel */

                if (!range_order(kernel_pfindex,
                                 kernel_pfindex + kernel_nframes,
                                 pfindex, pfindex + nframes))
                {
                        pfindex = kernel_pfindex + kernel_nframes;
                }

                /* check for intersection with modules */

                while (!pfoffset &&
                       ((pfindex + nframes) < (area_pfindex + area_nframes)))
                {

                        const struct multiboot_mod_list* mod =
                            (const struct multiboot_mod_list*)mb_info->mods_addr;

                        for (size_t j = 0; j < mb_info->mods_count; ++j, ++mod)
                        {
                                unsigned long mod_pfindex;
                                unsigned long mod_nframes;

                                mod_pfindex =
                                        pageframe_index((void *)mod->
                                                        mod_start);
                                mod_nframes =
                                        pageframe_count(mod->mod_end -
                                                        mod->mod_start);

                                /* check intersection */

                                if (range_order(mod_pfindex,
                                                mod_pfindex + mod_nframes,
                                                pfindex, pfindex + nframes))
                                {
                                        /* no intersection, offset found */
                                        pfoffset = pageframe_offset(pfindex);
                                }
                                else
                                {
                                        pfindex = mod_pfindex + mod_nframes;
                                }
                        }
                }
        }

        return pfoffset;
}

static int
multiboot_init_pmem(const struct multiboot_header *mb_header,
                       const struct multiboot_info *mb_info)
{
    unsigned long pfcount = pageframe_span(0, (mb_info->mem_upper + 1024) << 10);

    unsigned long pfindex = find_unused_area(mb_header,
                                             mb_info,
                                             pfcount >> PAGEFRAME_SHIFT);

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
        size_t i;
        unsigned long mmap_addr;

        mmap_addr = mb_info->mmap_addr;

        for (i = 0; i < mb_info->mmap_length;)
        {
                unsigned long pfindex;
                unsigned long nframes;

                const struct multiboot_mmap_entry* mmap =
                    (const struct multiboot_mmap_entry*)mmap_addr;

                pfindex = pageframe_index((void*)(uintptr_t)mmap->addr);
                nframes = pageframe_count(mmap->len);

                pmem_set_flags(pfindex,
                               nframes,
                               mmap->type == 1 ? PMEM_FLAG_USEABLE
                                               : PMEM_FLAG_RESERVED);

                mmap_addr += mmap->size + 4;
                i += mmap->size + 4;
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
        int err;
        const struct multiboot_mod_list* mod, *modend;

        err = 0;
        mod = (const struct multiboot_mod_list*)mb_info->mods_addr;
        modend = mod + mb_info->mods_count;

        while ((mod < modend)
               && ((err = multiboot_init_pmem_module(mod)) < 0))
        {
                ++mod;
        }

        return err;
}

static int
multiboot_load_modules(struct task *parent,
                       const struct multiboot_info *mb_info)
{
    const struct multiboot_mod_list* mod =
        (const struct multiboot_mod_list*)mb_info->mods_addr;

    for (size_t i = 0; i < mb_info->mods_count; ++i, ++mod) {

        size_t len = mod->mod_end - mod->mod_start + 1;

        execute_module(parent, mod->mod_start, len, (const char*)mod->cmdline);
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

    if (!(mb_info->flags & MULTIBOOT_INFO_MEMORY)) {
        return;
    }

    int res = multiboot_init_pmem(mb_header, mb_info);
    if (res < 0) {
        return;
    }

    pmem_set_flags(0, NPAGES, PMEM_FLAG_RESERVED);

    res = multiboot_init_pmem_kernel(mb_header);
    if (res < 0) {
        return;
    }

    if (mb_info->flags & MULTIBOOT_INFO_MEM_MAP) {
        res = multiboot_init_pmem_mmap(mb_info);
        if (res < 0) {
            return;
        }
    }

    if (mb_info->flags & MULTIBOOT_INFO_MODS) {
        res = multiboot_init_pmem_modules(mb_info);
        if (res < 0) {
            return;
        }
    }

    /* setup virtual memory and system services */

    struct task* task;
    res = general_init(&task, &stack);
    if (res < 0) {
        return;
    }

    /* load modules as ELF binaries */

    if (mb_info->flags & MULTIBOOT_INFO_MODS) {
        res = multiboot_load_modules(task, mb_info);
        if (res < 0) {
            return;
        }
    }
}
