/*
 *  oskernel - A small experimental operating-system kernel
 *  Copyright (C) 2009  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "types.h"
#include "multiboot.h"
#include "console.h"
#include "gdt.h"
#include "idt.h"
#include "intrrupt.h"
#include "pit.h"
#include "page.h"
#include "physmem.h"
#include "elfldr.h"

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
                 const struct multiboot_info *mb_info,
                 unsigned long npages)
{
        size_t i;
        unsigned long mmap_addr;
        unsigned long kernel_pgindex, kernel_npages;
        unsigned long pgoffset;

        kernel_pgindex = page_index(mb_header->load_addr);
        kernel_npages = page_count(mb_header->bss_end_addr -
                                   mb_header->load_addr);

        mmap_addr = mb_info->mmap_addr;

        for (pgoffset = 0, i = 0; !pgoffset && (i < mb_info->mmap_length);) {
                const struct multiboot_mmap *mmap;
                unsigned long area_pgindex;
                unsigned long area_npages;
                unsigned long pgindex;

                mmap = (const struct multiboot_mmap*)mmap_addr;

                /* next entry address */
                mmap_addr += mmap->size+4;
                i += mmap->size+4;

                /* area if not useable */
                if (mmap->type != 1) {
                        continue;
                }

                /* area page index and length */

                area_pgindex = page_index(
                        (((unsigned long long)mmap->base_addr_high)<<32) +
                                              mmap->base_addr_low);

                area_npages = page_count(
                        (((unsigned long long)mmap->length_high)<<32) +
                                              mmap->length_low);

                /* area at address 0, or too small */
                if (!area_pgindex || (area_npages < npages)) {
                        continue;
                }

                /* possible index */
                pgindex = area_pgindex;

                /* check for intersection with kernel */
                if (!range_order(kernel_pgindex,
                                 kernel_pgindex+kernel_npages,
                                 pgindex,
                                 pgindex+npages)) {
                        pgindex = kernel_pgindex+kernel_npages;
                }

                /* check for intersection with modules */

                while ( !pgoffset &&
                       ((pgindex+npages) < (area_pgindex+area_npages)) ) {

                        size_t j;
                        const struct multiboot_module *mod;

                        mod = (const struct multiboot_module*)mb_info->mods_addr;

                        for (j = 0; j < mb_info->mods_count; ++j, ++mod) {
                                unsigned long mod_pgindex;
                                unsigned long mod_npages;

                                mod_pgindex = page_index(mod->mod_start);
                                mod_npages  = page_count(mod->mod_end -
                                                         mod->mod_start);

                                /* check intersection */
                                if (range_order(mod_pgindex,
                                                mod_pgindex+mod_npages,
                                                pgindex,
                                                pgindex+npages)) {
                                        /* no intersection, offset found */
                                        pgoffset = page_offset(pgindex);
                                } else {
                                        pgindex = mod_pgindex+mod_npages;
                                }
                        }
                }
        }

        return pgoffset;
}

static int
init_physmem(const struct multiboot_header *mb_header,
             const struct multiboot_info *mb_info)
{
        unsigned long physmap, npages;

        npages  = page_count((mb_info->mem_upper+1)<<10);
        physmap = find_unused_area(mb_header, mb_info, npages>>PAGE_SHIFT);

        console_printf("found phymap area at %x\n", (unsigned long)physmap);

        /* init memory manager */
        return physmem_init(physmap, npages);
}

static int
init_physmap_kernel(const struct multiboot_header *mb_header)
{
        unsigned long pgindex, npages;

        pgindex = page_index(mb_header->load_addr);
        npages  = page_count(mb_header->bss_end_addr-mb_header->load_addr);

        return physmem_add_area(pgindex, npages, PHYSMEM_FLAG_RESERVED);
}

static int
init_physmap_areas(const struct multiboot_info *mb_info)
{
        size_t i;
        unsigned long mmap_addr;

        mmap_addr = mb_info->mmap_addr;

        /* add memory areas */

/*        console_printf("mmap_length=%x\n", mb_info->mmap_length);*/

        for (i = 0; i < mb_info->mmap_length;) {
                const struct multiboot_mmap *mmap;
                unsigned long pgoffset;
                unsigned long npages;

                mmap = (const struct multiboot_mmap*)mmap_addr;

                pgoffset = page_index(
                        (((unsigned long long)mmap->base_addr_high)<<32) +
                                              mmap->base_addr_low);

                npages = page_count(
                        (((unsigned long long)mmap->length_high)<<32) +
                                              mmap->length_low);

/*                console_printf("%s:%x %x %x %x\n",
                                __FILE__,
                                __LINE__,
                                (pgoffset<<PHYSPAGE_SHIFT),
                                (npages<<PHYSPAGE_SHIFT),
                                mmap->type);*/

                physmem_add_area(pgoffset,
                                 npages,
                                 mmap->type==1 ? PHYSMEM_FLAG_USEABLE
                                               : PHYSMEM_FLAG_RESERVED);

                mmap_addr += mmap->size+4;
                i += mmap->size+4;
        }

        return 0;
}

static int
init_physmap_modules(const struct multiboot_info *mb_info)
{
        size_t i;
        const struct multiboot_module *mod;

        mod = (const struct multiboot_module*)mb_info->mods_addr;

        for (i = 0; i < mb_info->mods_count; ++i, ++mod) {
                unsigned long pgoffset;
                unsigned long npages;

                pgoffset = page_index(mod->mod_start);
                npages   = page_count(mod->mod_end-mod->mod_start);

/*                console_printf("%s:%x %x %x\n",
                                __FILE__,
                                __LINE__,
                                (pgoffset<<PHYSPAGE_SHIFT),
                                (npages<<PHYSPAGE_SHIFT));*/

                physmem_add_area(pgoffset,
                                 npages,
                                 PHYSMEM_FLAG_RESERVED);
        }

        return 0;
}

static int
load_modules(const struct multiboot_info *mb_info)
{
        size_t i;
        const struct multiboot_module *mod;

        mod = (const struct multiboot_module*)mb_info->mods_addr;

        for (i = 0; i < mb_info->mods_count; ++i, ++mod) {
/*                console_printf("found module at %x, size=%x\n",
                                mod->mod_start,
                                mod->mod_end-mod->mod_start);*/

                elf_exec((const void*)mod->mod_start);
        }

        return 0;
}

#include "pte.h"
#include "pde.h"
#include "virtmem.h"
#include "tcb.h"
#include "task.h"

static int
build_init_task(void)
{
        int err;
        struct page_directory *pd;
        struct task *task;
        unsigned long pgindex;
        unsigned long npages;

        /* create virtual address space */

        if ( !(pd = page_directory_create()) ) {
                err = -1;
                goto err_page_directory_create;
        }

        /* populate kernel area */

        pgindex = page_index(g_min_kernel_virtaddr);
        npages  = page_count(g_max_kernel_virtaddr+g_min_kernel_virtaddr);

        if ((err = page_directory_install_page_tables(pd, pgindex, npages)) < 0) {
                goto err_page_directory_install_page_tables;
        }

        /* create thread (0:0) */

        if ( !(task = task_lookup(0)) ) {
                err = -1;
                goto err_task_lookup;
        }

        /* load save registers in TCB 0 */


        return 0;

err_task_lookup:
err_page_directory_install_page_tables:
        page_directory_destroy(pd);
err_page_directory_create:
        return err;
}

unsigned long tickcounter = 0;

void
multiboot_main(const struct multiboot_header *mb_header,
               const struct multiboot_info *mb_info)
{
/*        struct tcb tcb;
        struct task task;*/

/*        unsigned long esp;*/

        console_printf("%s...\n\t%s\n", "OS kernel booting",
                                        "Cool, isn't it?");

/*        console_printf("Multiboot magic %x\n", mb_header->magic);

        console_printf("%x %x %x\n", mb_header->load_addr,
                                     mb_header->load_end_addr,
                                     mb_header->bss_end_addr);*/

        if (mb_info->flags&MULTIBOOT_INFO_FLAG_MEM) {
                init_physmem(mb_header, mb_info);
        } else {
                /* FIXME: abort kernel */
        }

        init_physmap_kernel(mb_header);

        if (mb_info->flags&MULTIBOOT_INFO_FLAG_MMAP) {
                init_physmap_areas(mb_info);
                init_physmap_modules(mb_info);
        }

        /* setup GDT for protected mode */
        gdt_init();
        gdt_install();

        /* setup IDT for protected mode */
        idt_init();
        idt_install();
        idt_install_irq();

        /* setup PIT for system timer */
/*        pit_install(0, 20, PIT_MODE_RATEGEN);*/

        if (mb_info->flags&MULTIBOOT_INFO_FLAG_MODS) {
                load_modules(mb_info);
        }

/*        sti();*/

/*        for (;;) {*/
/*                __asm__("int $0x20\n\t");*/
/*                cli();
                __asm__("movl %%esp, %0\n\t"
                                : "=r"(esp)
                                :
                                :);

                crt_setpos(12, 40);
                console_printf("%x", esp);
                sti();
        }*/
}

