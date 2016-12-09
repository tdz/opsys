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
#include <stdbool.h>
#include <stddef.h>
#include "console.h"
#include "drivers/multiboot_vga/multiboot_vga.h"
#include "main.h"
#include "page.h"
#include "pageframe.h"
#include "pmem.h"

/*
 * Platform drivers that depend in Multiboot
 */

static struct multiboot_vga_drv g_mb_vga_drv;

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

/*
 * PMEM initialization
 *
 * PMEM requires a memory map to keep track of the available page
 * frames. This map must be allocated from available page frames,
 * which results in a chicken-and-egg problem.
 *
 * To get an idea of the map size, we calculate the size of the
 * available memory, from the Multiboot mmap entries. To allocate
 * the memory map, we search the mmap entries for available space,
 * carefully avoiding locations that are occupied by the kernel or
 * modules.
 *
 * We give the allocated map to PMEM and mark all pre-allocated
 * areas, such as kernel and module frames, using PMEM interfaces.
 */

static bool
overlaps_with(unsigned long beg1, unsigned long len1,
              unsigned long beg2, unsigned long len2)
{
    unsigned long end1 = beg1 + len1;
    unsigned long end2 = beg2 + len2;

    return !((end1 <= beg2) || (end2 <= beg1));
}

static bool
is_within(unsigned long inner_beg, unsigned long inner_len,
          unsigned long outer_beg, unsigned long outer_len)
{
    unsigned long inner_end = inner_beg + inner_len;
    unsigned long outer_end = outer_beg + outer_len;

    return (inner_beg >= outer_beg) && (inner_end <= outer_end);
}

static uintptr_t
find_unused_area(const struct multiboot_header* header,
                 const struct multiboot_info* info,
                 unsigned long nframes)
{
    os_index_t kernel_pfindex =
        pageframe_index((void*)(uintptr_t)header->load_addr);

    size_t kernel_nframes =
        pageframe_span((void*)(uintptr_t)header->load_addr,
                       header->bss_end_addr - header->load_addr + 1);

    for (const struct multiboot_mmap_entry* mmap =
                first_mmap_entry_of_type(info, MULTIBOOT_MEMORY_AVAILABLE);
            mmap;
            mmap = next_mmap_entry_of_type(info, mmap, MULTIBOOT_MEMORY_AVAILABLE)) {

        /* area page index and length */

        os_index_t mmap_pfindex = pageframe_index((void*)(uintptr_t)mmap->addr);
        size_t mmap_nframes = pageframe_count(mmap->len);

        /* area at address 0, or too small */
        if (!mmap_pfindex || (mmap_nframes < nframes)) {
            continue;
        }

        /* possible index */
        os_index_t pfindex = mmap_pfindex;

        /* check for intersection with kernel */

        if (overlaps_with(pfindex, nframes, kernel_pfindex, kernel_nframes)) {
            pfindex = kernel_pfindex + kernel_nframes;
        }

        /* check for intersection with modules */

        for (const struct multiboot_mod_list* mod = first_mod_list(info);
                mod;
                mod = next_mod_list(info, mod)) {

            os_index_t mod_pfindex =
                pageframe_index((void *)mod->mod_start);

            size_t mod_nframes =
                pageframe_count(mod->mod_end - mod->mod_start + 1);

            if (overlaps_with(pfindex, nframes, mod_pfindex, mod_nframes)) {
                pfindex = mod_pfindex + mod_nframes;
            }
        }

        if (is_within(pfindex, nframes, mmap_pfindex, mmap_nframes)) {
            /* found an unused area within mmap entry */
            return pageframe_offset(pfindex);
        }
    }

    return 0; /* not found */
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
mark_mmap_areas(const struct multiboot_info* info)
{
    for (const struct multiboot_mmap_entry* mmap = first_mmap_entry(info);
            mmap;
            mmap = next_mmap_entry(info, mmap)) {

        os_index_t pfindex = pageframe_index((void*)(uintptr_t)mmap->addr);
        size_t nframes = pageframe_count(mmap->len);

        unsigned char flags = mmap->type == MULTIBOOT_MEMORY_AVAILABLE ?
                                PMEM_FLAG_USEABLE :
                                PMEM_FLAG_RESERVED;

        int res = pmem_set_flags(pfindex, nframes, flags);
        if (res < 0) {
            return res;
        }
    }

    return 0;
}

static int
claim_multiboot_frames(const struct multiboot_header* header,
                       const struct multiboot_info* info)
{
    int res = pmem_claim_frames(pageframe_index(header),
                                pageframe_span(header, sizeof(*header)));
    if (res < 0) {
        return res;
    }

    res = pmem_claim_frames(pageframe_index(info),
                            pageframe_span(info, sizeof(*info)));
    if (res < 0) {
        return res;
    }

    const struct multiboot_mmap_entry* mmap = first_mmap_entry(info);

    if (mmap) {
        res = pmem_claim_frames(pageframe_index(mmap),
                                pageframe_span(mmap, info->mmap_length));
        if (res < 0) {
            return res;
        }
    }

    const struct multiboot_mod_list* mod = first_mod_list(info);

    if (mod) {
        res = pmem_claim_frames(pageframe_index(mod),
                                pageframe_span(mod, info->mods_count * sizeof(*mod)));
        if (res < 0) {
            return res;
        }
    }

    return 0;
}

static int
claim_kernel_frames(const struct multiboot_header* header)
{
    return pmem_claim_frames(pageframe_index((void*)(uintptr_t)header->load_addr),
                             pageframe_count(header->bss_end_addr -
                                             header->load_addr));
}

static int
claim_modules_frames(const struct multiboot_info* info)
{
    for (const struct multiboot_mod_list* mod = first_mod_list(info);
            mod;
            mod = next_mod_list(info, mod)) {

        os_index_t pfindex = pageframe_index((void*)(uintptr_t)mod->mod_start);
        size_t nframes = pageframe_count(mod->mod_end - mod->mod_start);

        int res = pmem_claim_frames(pfindex, nframes);
        if (res < 0) {
            return res;
        }
    }

    return 0;
}

static int
claim_stack_frames(const void* stack)
{
    return pmem_claim_frames(pageframe_index(stack), 1);
}

static int
init_pmem_from_multiboot(const struct multiboot_header* header,
                         const struct multiboot_info* info,
                         const void* stack)
{
    const static size_t NPAGES = (4 * 1024 * 1024) / PAGE_SIZE;

    /* init physical memory */

    size_t mem = available_mem(info);
    if (!mem) {
        return -ENOMEM;
    }

    unsigned long pfcount = pageframe_span(0, mem);
    unsigned long pfindex = find_unused_area(header,
                                             info,
                                             pfcount >> PAGEFRAME_SHIFT);
    if (!pfindex) {
        return -ENOMEM;
    }

    pmem_map_t* memmap = (pmem_map_t*)(uintptr_t)pfindex;

    int res = pmem_init(memmap, pfcount);
    if (res < 0) {
        return res;
    }

    res = mark_mmap_areas(info);
    if (res < 0) {
        return res;
    }

    /* claim memory map; global variables of pmem claimed
     * by kernel image */
    res = pmem_claim_frames(pageframe_index(memmap),
                            pageframe_count(pfcount * sizeof(*memmap)));
    if (res < 0) {
        return res;
    }

    /* claim frames with lowest 4 MiB reserved for DMA,
     * kernel, etc */

    res = pmem_claim_frames(pageframe_index(0), NPAGES);
    if (res < 0) {
        return res;
    }

    res = claim_multiboot_frames(header, info);
    if (res < 0) {
        return res;
    }

    res = claim_kernel_frames(header);
    if (res < 0) {
        return res;
    }

    res = claim_modules_frames(info);
    if (res < 0) {
        return res;
    }

    res = claim_stack_frames(stack);
    if (res < 0) {
        return res;
    }

    return 0;
}

/*
 * Module loader
 */

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

/*
 * Entry point
 */

/**
 * The function |multiboot_init| is called from multiboot.S as the entry
 * point into the C code. Don't declare it 'static.'
 */
void
multiboot_init(const struct multiboot_header* header,
               const struct multiboot_info* info,
               void* stack)
{
    /* init VGA and console */

    int res = multiboot_vga_init(&g_mb_vga_drv,
                                 header->width,
                                 header->height);
    if (res < 0) {
        return;
    }

    res = init_console(&g_mb_vga_drv.drv);
    if (res < 0) {
        return;
    }

    /* At this point we have a console ready, so display
     * something to the user. */
    console_printf("opsys booting...\n");

    /* init physical memory */

    res = init_pmem_from_multiboot(header, info, stack);
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

    res = multiboot_load_modules(task, info);
    if (res < 0) {
        return;
    }
}
