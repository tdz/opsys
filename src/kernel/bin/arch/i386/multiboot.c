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
#include <string.h>
#include "console.h"
#include "drivers/multiboot_vga/multiboot_vga.h"
#include "elf.h"
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
 * ELF-segment iteration
 */

static const Elf32_Shdr*
first_elf32_shdr(const struct multiboot_info* info)
{
    if (!(info->flags & MULTIBOOT_INFO_ELF_SHDR)) {
        return NULL;
    }
    return (const Elf32_Shdr*)(uintptr_t)info->u.elf_sec.addr;
}

static const Elf32_Shdr*
next_elf32_shdr(const struct multiboot_info* info, const Elf32_Shdr* shdr)
{
    const Elf32_Shdr* first =
        (const Elf32_Shdr*)(uintptr_t)info->u.elf_sec.addr;

    ++shdr;

    if (info->u.elf_sec.num <= shdr - first) {
        return NULL;
    }
    return shdr;
}

static const Elf32_Shdr*
next_elf32_shdr_in_memory(const struct multiboot_info* info,
                          const Elf32_Shdr* shdr)
{
    do {
        shdr = next_elf32_shdr(info, shdr);
    } while (shdr && !shdr->sh_addr);

    return shdr; // either memory-mapped section or NULL
}

static const Elf32_Shdr*
first_elf32_shdr_in_memory(const struct multiboot_info* info)
{
    const Elf32_Shdr* shdr = first_elf32_shdr(info);

    if (shdr && shdr->sh_addr) {
        return shdr;
    }
    return next_elf32_shdr_in_memory(info, shdr);
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
find_unused_area(const struct multiboot_info* info,
                 unsigned long nframes)
{
    for (const struct multiboot_mmap_entry* mmap =
                first_mmap_entry_of_type(info, MULTIBOOT_MEMORY_AVAILABLE);
            mmap;
            mmap = next_mmap_entry_of_type(info, mmap, MULTIBOOT_MEMORY_AVAILABLE)) {

        /* area page index and length */

        os_index_t mmap_pfindex = pageframe_index((void*)(uintptr_t)mmap->addr);
        size_t     mmap_nframes = pageframe_span((void*)(uintptr_t)mmap->addr, mmap->len);

        /* area at address 0, or too small */
        if (!mmap_pfindex || (mmap_nframes < nframes)) {
            continue;
        }

        /* possible index */
        os_index_t pfindex = mmap_pfindex;

        /* check for intersection with kernel */

        for (const Elf32_Shdr* shdr = first_elf32_shdr_in_memory(info);
                shdr;
                shdr = next_elf32_shdr_in_memory(info, shdr)) {

            const void* addr = (const void*)(uintptr_t)shdr->sh_addr;

            os_index_t shdr_pfindex = pageframe_index(addr);
            size_t     shdr_nframes = pageframe_span(addr, shdr->sh_size);

            if (overlaps_with(pfindex, nframes, shdr_pfindex, shdr_nframes)) {
                pfindex = shdr_pfindex + shdr_nframes;
            }
        }

        /* check for intersection with modules */

        for (const struct multiboot_mod_list* mod = first_mod_list(info);
                mod;
                mod = next_mod_list(info, mod)) {

            os_index_t mod_pfindex =
                pageframe_index((void *)(uintptr_t)mod->mod_start);

            size_t mod_nframes =
                pageframe_span((void *)(uintptr_t)mod->mod_start,
                               mod->mod_end - mod->mod_start + 1);

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

static unsigned long
available_frames(const struct multiboot_info* info)
{
    unsigned long nframes = 0;

    for (const struct multiboot_mmap_entry* mmap =
                first_mmap_entry_of_type(info, MULTIBOOT_MEMORY_AVAILABLE);
            mmap;
            mmap = next_mmap_entry_of_type(info, mmap, MULTIBOOT_MEMORY_AVAILABLE)) {

        unsigned long end = pageframe_index((void*)(uintptr_t)mmap->addr) +
                            pageframe_span((void*)(uintptr_t)mmap->addr, mmap->len);

        if (end > nframes) {
            nframes = end;
        }
    }

    return nframes;
}

static pmem_map_t*
alloc_memmap(const struct multiboot_info* info,
             unsigned long pfcount)
{
    unsigned long nframes = pageframe_count(pfcount * sizeof(pmem_map_t));

    unsigned long pfindex = find_unused_area(info, nframes);
    if (!pfindex) {
        return NULL;
    }

    return (pmem_map_t*)(uintptr_t)pfindex;
}

static int
mark_mmap_areas(const struct multiboot_info* info)
{
    for (const struct multiboot_mmap_entry* mmap =
                first_mmap_entry_of_type(info, MULTIBOOT_MEMORY_AVAILABLE);
            mmap;
            mmap = next_mmap_entry_of_type(info, mmap, MULTIBOOT_MEMORY_AVAILABLE)) {

        os_index_t pfindex = pageframe_index((void*)(uintptr_t)mmap->addr);
        size_t     nframes = pageframe_span((void*)(uintptr_t)mmap->addr, mmap->len);

        int res = pmem_set_type(pfindex, nframes, PMEM_TYPE_AVAILABLE);
        if (res < 0) {
            return res;
        }
    }

    return 0;
}

static int
claim_multiboot_frames(const struct multiboot_info* info)
{
    int res = pmem_claim_frames(pageframe_index(info),
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
claim_kernel_frames(const struct multiboot_info* info)
{
    /* kernel includes initial stack and Multiboot header */

    for (const Elf32_Shdr* shdr = first_elf32_shdr_in_memory(info);
            shdr;
            shdr = next_elf32_shdr_in_memory(info, shdr)) {

        const void* addr = (const void*)(uintptr_t)shdr->sh_addr;

        os_index_t pfindex = pageframe_index(addr);
        size_t     nframes = pageframe_span(addr, shdr->sh_size);

        int res = pmem_claim_frames(pfindex, nframes);
        if (res < 0) {
            return res;
        }
    }

    return 0;
}

static int
claim_modules_frames(const struct multiboot_info* info)
{
    for (const struct multiboot_mod_list* mod = first_mod_list(info);
            mod;
            mod = next_mod_list(info, mod)) {

        os_index_t pfindex = pageframe_index((void*)(uintptr_t)mod->mod_start);
        size_t     nframes = pageframe_span((void*)(uintptr_t)mod->mod_start,
                                            mod->mod_end - mod->mod_start + 1);

        int res = pmem_claim_frames(pfindex, nframes);
        if (res < 0) {
            return res;
        }

        const char* cmdline = (const char*)(uintptr_t)mod->cmdline;

        if (cmdline) {
            pfindex = pageframe_index(cmdline);
            nframes = pageframe_span(cmdline, strlen(cmdline));

            res = pmem_claim_frames(pfindex, nframes);
            if (res < 0) {
                return res;
            }
        }
    }

    return 0;
}

static int
init_pmem_from_multiboot(const struct multiboot_info* info)
{
    const static size_t NPAGES = (4 * 1024 * 1024) / PAGE_SIZE;

    /* init physical memory */

    unsigned long pfcount = available_frames(info);
    if (!pfcount) {
        return -ENOMEM;
    }

    pmem_map_t* memmap = alloc_memmap(info, pfcount);
    if (!memmap) {
        return -ENOMEM;
    }

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
                            pageframe_span(memmap, pfcount * sizeof(*memmap)));
    if (res < 0) {
        return res;
    }

    /* claim frames with lowest 4 MiB reserved for DMA,
     * kernel, etc */

    res = pmem_claim_frames(pageframe_index(0), NPAGES);
    if (res < 0) {
        return res;
    }

    res = claim_multiboot_frames(info);
    if (res < 0) {
        return res;
    }

    res = claim_kernel_frames(info);
    if (res < 0) {
        return res;
    }

    res = claim_modules_frames(info);
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

    res = init_pmem_from_multiboot(info);
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
