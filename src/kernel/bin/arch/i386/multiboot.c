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
#include "alloc.h"
#include "console.h"
#include "drivers/i8042/kbd.h"
#include "drivers/i8254/i8254.h"
#include "drivers/i8259/pic.h"
#include "drivers/multiboot_vga/multiboot_vga.h"
#include "idt.h"
#include "interupt.h"
#include "iomem.h"
#include "elf.h"
#include "gdt.h"
#include "page.h"
#include "pagedir.h"
#include "pageframe.h"
#include "pagetbl.h"
#include "pmem.h"
#include "sched.h" // for SCHED_FREQ
#include "syscall.h"
#include "sysexec.h"
#include "vmem.h"
#include "vmemhlp.h"

/*
 * Virtual address space for kernel task
 */

static struct vmem g_kernel_vmem;

/*
 * Platform-specific drivers
 */

static struct i8254_drv         g_i8254_drv;
static struct multiboot_vga_drv g_mb_vga_drv;

/*
 * Platform entry points for ISR handlers
 *
 * The platform_ functions below are the entry points from the
 * IDT's interupt handlers into the main executable. The functions
 * must forward the interupts to whatever drivers or modules have
 * been initialized.
 */

void __attribute__((used))
platform_handle_invalid_opcode(void* ip)
{
    console_printf("invalid opcode ip=%x.\n", (unsigned long)ip);
}

void __attribute__((used))
platform_handle_irq(unsigned char irqno)
{
    pic_handle_irq(irqno);
}

void __attribute__((used))
platform_eoi(unsigned char irqno)
{
    pic_eoi(irqno);
}

void __attribute__((used))
platform_handle_segmentation_fault(void* ip)
{
    vmem_segfault_handler(ip);
}

void __attribute__((used))
platform_handle_page_fault(void* ip, void* addr, unsigned long errcode)
{
    vmem_pagefault_handler(ip, addr, errcode);
}

void __attribute__((used))
platform_handle_syscall(unsigned long* r0, unsigned long* r1,
                        unsigned long* r2, unsigned long* r3)
{
    syscall_entry_handler(r0, r1, r2, r3);
}

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

    /* claim Multiboot data structures */

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
 * VMEM
 */

static struct page_directory*
create_page_directory(void)
{
    unsigned long nframes = pageframe_count(sizeof(struct page_directory));

    unsigned long pfindex = pmem_alloc_frames(nframes);
    if (!pfindex) {
        return NULL;
    }

    struct page_directory* pd =
        (struct page_directory*)pageframe_offset(pfindex);

    int res = page_directory_init(pd);
    if (res < 0) {
        goto err_page_directory_init;
    }

    return pd;

err_page_directory_init:
    pmem_unref_frames(pfindex, nframes);
    return NULL;
}

static void
destroy_page_directory(struct page_directory* pd)
{
    page_directory_uninit(pd);
    pmem_unref_frames(pageframe_index(pd), pageframe_count(sizeof(*pd)));
}

int
map_page_directory(struct page_directory* pd, struct vmem* vmem)
{
    /* So far the page directory is only allocated in physical
     * memory, but not mapped into the virtual address space. We
     * have to do this _before_ we enable paging.
     *
     * Here we create an identity mapping for page-directory's page
     * frame. Whis the address stored in vmem remains valid.
     */
    int res = vmem_map_pageframes_nopg(vmem,
                                       pageframe_index(pd),
                                       page_index(pd),
                                       pageframe_count(sizeof(*pd)),
                                       PTE_FLAG_PRESENT | PTE_FLAG_WRITEABLE);
    if (res < 0) {
        return res;
    }
    return 0;
}

int
map_memmap_frames(struct vmem* vmem, const pmem_map_t* memmap, size_t nframes)
{
    int res = vmem_map_pageframes_nopg(vmem,
                                       pageframe_index(memmap),
                                       pageframe_index(memmap),
                                       pageframe_count(nframes * sizeof(*memmap)),
                                       PTE_FLAG_PRESENT |
                                       PTE_FLAG_WRITEABLE);
    if (res < 0) {
        return res;
    }
    return 0;
}

int
map_multiboot_frames(struct vmem* vmem, const struct multiboot_info* info)
{
    int res = vmem_map_pageframes_nopg(vmem,
                                       pageframe_index(info),
                                       pageframe_index(info),
                                       pageframe_span(info, sizeof(*info)),
                                       PTE_FLAG_PRESENT |
                                       PTE_FLAG_WRITEABLE);
    if (res < 0) {
        return res;
    }

    const struct multiboot_mmap_entry* mmap = first_mmap_entry(info);

    if (mmap) {
        res = vmem_map_pageframes_nopg(vmem,
                                       pageframe_index(mmap),
                                       pageframe_index(mmap),
                                       pageframe_span(mmap, info->mmap_length),
                                       PTE_FLAG_PRESENT |
                                       PTE_FLAG_WRITEABLE);
        if (res < 0) {
            return res;
        }
    }

    const struct multiboot_mod_list* mod = first_mod_list(info);

    if (mod) {
        res = vmem_map_pageframes_nopg(vmem,
                                       pageframe_index(mod),
                                       pageframe_index(mod),
                                       pageframe_span(mod, info->mods_count * sizeof(*mod)),
                                       PTE_FLAG_PRESENT |
                                       PTE_FLAG_WRITEABLE);
        if (res < 0) {
            return res;
        }
    }

    return 0;
}

static int
map_kernel_frames(struct vmem* vmem, const struct multiboot_info* info)
{
    /* kernel includes initial stack and Multiboot header */

    for (const Elf32_Shdr* shdr = first_elf32_shdr_in_memory(info);
            shdr;
            shdr = next_elf32_shdr_in_memory(info, shdr)) {

        const void* addr = (const void*)(uintptr_t)shdr->sh_addr;

        os_index_t pfindex = pageframe_index(addr);
        size_t     nframes = pageframe_span(addr, shdr->sh_size);

        int res = vmem_map_pageframes_nopg(vmem,
                                           pfindex, pfindex, nframes,
                                           PTE_FLAG_PRESENT |
                                           PTE_FLAG_WRITEABLE);
        if (res < 0) {
            return res;
        }
    }

    return 0;
}

static int
map_modules_frames(struct vmem* vmem, const struct multiboot_info* info)
{
    for (const struct multiboot_mod_list* mod = first_mod_list(info);
            mod;
            mod = next_mod_list(info, mod)) {

        os_index_t pfindex = pageframe_index((void*)(uintptr_t)mod->mod_start);
        size_t nframes = pageframe_count(mod->mod_end - mod->mod_start);

        int res = vmem_map_pageframes_nopg(vmem,
                                           pfindex, pfindex, nframes,
                                           PTE_FLAG_PRESENT |
                                           PTE_FLAG_WRITEABLE);
        if (res < 0) {
            return res;
        }

        const char* cmdline = (const char*)(uintptr_t)mod->cmdline;

        if (cmdline) {
            pfindex = pageframe_index(cmdline);
            nframes = pageframe_span(cmdline, strlen(cmdline));

            res = vmem_map_pageframes_nopg(vmem,
                                           pfindex, pfindex, nframes,
                                           PTE_FLAG_PRESENT |
                                           PTE_FLAG_WRITEABLE);
            if (res < 0) {
                return res;
            }
        }
    }

    return 0;
}

static int
init_vmem_from_multiboot(struct vmem* vmem, const struct multiboot_info* info)
{
    /* init vmem data structure for kernel task */

    struct page_directory* pd = create_page_directory();
    if (!pd) {
        return -ENOMEM;
    }

    int res = vmem_init(vmem, PAGING_32BIT, pd);
    if (res < 0) {
        goto err_vmem_init;
    }

    /* install page tables in all kernel areas */

    enum vmem_area_name name;

    for (name = 0; name < LAST_VMEM_AREA; ++name) {

        const struct vmem_area* area = vmem_area_get_by_name(name);

        if (!(area->flags & VMEM_AREA_FLAG_PAGETABLES)) {
            continue;
        }

        unsigned long ptindex = pagetable_index(page_address(area->pgindex));
        unsigned long ptcount = pagetable_count(page_address(area->pgindex),
                                                page_memory(area->npages));

        res = vmem_alloc_page_tables_nopg(vmem, ptindex, ptcount,
                                          PDE_FLAG_PRESENT|
                                          PDE_FLAG_WRITEABLE);

        if (res < 0) {
            goto err_vmem_alloc_page_tables_nopg;
        }
    }

    /* create identity mapping for all identity areas */

    for (name = 0; name < LAST_VMEM_AREA; ++name) {

        const struct vmem_area *area = vmem_area_get_by_name(name);

        if (!(area->flags & VMEM_AREA_FLAG_POLUTE)) {
            continue;
        } else if (!(area->flags & VMEM_AREA_FLAG_IDENTITY)) {
            continue;
        }

        res = vmem_map_pageframes_nopg(vmem,
                                       area->pgindex,
                                       area->pgindex,
                                       area->npages,
                                       PTE_FLAG_PRESENT |
                                       PTE_FLAG_WRITEABLE);
        if (res < 0) {
            goto err_vmem_map_pageframes_nopg_identity;
        }
    }

    /* populate remaining areas */

    for (name = 0; name < LAST_VMEM_AREA; ++name) {

        const struct vmem_area *area = vmem_area_get_by_name(name);

        if (!(area->flags & VMEM_AREA_FLAG_POLUTE)) {
            continue;
        } else if (area->flags & VMEM_AREA_FLAG_IDENTITY) {
            continue;
        }

        os_index_t pfindex =
            pmem_alloc_frames(pageframe_count(page_memory(area->npages)));

        if (!pfindex) {
            res = -ENOMEM;
            break;
        }

        res = vmem_map_pageframes_nopg(vmem, pfindex,
                                       area->pgindex,
                                       area->npages,
                                       PTE_FLAG_PRESENT |
                                       PTE_FLAG_WRITEABLE);
        if (res < 0) {
            goto err_vmem_map_pageframes_nopg_pollute;
        }
    }

    /* map page directory */
    res = map_page_directory(pd, vmem);
    if (res < 0){
        goto err_map_page_directory;
    }

    res = map_memmap_frames(vmem, pmem_get_memmap(), pmem_get_nframes());
    if (res < 0){
        goto err_map_memmap_frames;
    }

    res = map_multiboot_frames(vmem, info);
    if (res < 0){
        goto err_map_multiboot_frames;
    }

    res = map_kernel_frames(vmem, info);
    if (res < 0){
        goto err_map_kernel_frames;
    }

    res = map_modules_frames(vmem, info);
    if (res < 0){
        goto err_map_modules_frames;
    }

    /* prepare temporary mappings */

    res = vmem_install_tmp_nopg(vmem);
    if (res < 0) {
        goto err_vmem_install_tmp_nopg;
    }

    return 0;

err_vmem_install_tmp_nopg:
err_map_modules_frames:
err_map_kernel_frames:
err_map_multiboot_frames:
err_map_memmap_frames:
err_map_page_directory:
err_vmem_map_pageframes_nopg_pollute:
err_vmem_map_pageframes_nopg_identity:
err_vmem_alloc_page_tables_nopg:
    vmem_uninit(vmem);
err_vmem_init:
    destroy_page_directory(pd);
    return res;
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
    /* init VGA and console
     */

    int res = multiboot_vga_early_init(&g_mb_vga_drv,
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

    /* init memory
     */

    gdt_init();
    gdt_install();

    res = init_pmem_from_multiboot(info);
    if (res < 0) {
        return;
    }

    res = init_vmem_from_multiboot(&g_kernel_vmem, info);
    if (res < 0) {
        return;
    }

    res = init_iomem(&g_kernel_vmem);
    if (res < 0) {
        return;
    }

    res = multiboot_vga_late_init(&g_mb_vga_drv);
    if (res < 0) {
        return;
    }

    vmem_enable(&g_kernel_vmem); // paging enabled! avoid PMEM after this point

    res = allocator_init(&g_kernel_vmem);
    if (res < 0) {
        return;
    }

    /* init interupt controller and handling */

    init_idt();
    pic_install();

    /* setup PIT for system timer */

    res = i8254_init(&g_i8254_drv);
    if (res < 0) {
        return;
    }

    i8254_install_timer(&g_i8254_drv, SCHED_FREQ); // TODO: avoid SCHED_FREQ

    /* init keyboard; TODO: this driver should run as a user-space program */
    res = kbd_init();
    if (res < 0) {
        return;
    }

    /* setup system services */

    struct task* task;
    res = schedule_kernel_threads(&g_kernel_vmem, &stack, &task);
    if (res < 0) {
        return;
    }

    sti(); // TODO: should be enabled right after pic_install()

    /* load modules as ELF binaries */

    res = multiboot_load_modules(task, info);
    if (res < 0) {
        return;
    }
}
