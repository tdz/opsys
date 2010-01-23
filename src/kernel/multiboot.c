/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009-2010  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "stddef.h"
#include "errno.h"
#include "types.h"
#include "multiboot.h"
#include "console.h"
#include "gdt.h"
#include "idt.h"
#include "interupt.h"
#include "pic.h"
#include "pit.h"
#include "pageframe.h"
#include "physmem.h"

#include "pde.h"
#include "pagedir.h"
#include "virtmem.h"

#include "elfldr.h"

#include "kbd.h"

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
                 unsigned long nframes)
{
        size_t i;
        unsigned long mmap_addr;
        unsigned long kernel_pfindex, kernel_nframes;
        unsigned long pfoffset;

        kernel_pfindex = pageframe_index(mb_header->load_addr);
        kernel_nframes = pageframe_span(mb_header->load_addr,
                                        mb_header->bss_end_addr -
                                        mb_header->load_addr+1);

        mmap_addr = mb_info->mmap_addr;

        for (pfoffset = 0, i = 0; !pfoffset && (i < mb_info->mmap_length);) {
                const struct multiboot_mmap *mmap;
                unsigned long area_pfindex;
                unsigned long area_nframes;
                unsigned long pfindex;

                mmap = (const struct multiboot_mmap*)mmap_addr;

                /* next entry address */
                mmap_addr += mmap->size+4;
                i += mmap->size+4;

                /* area if not useable */
                if (mmap->type != 1) {
                        continue;
                }

                /* area page index and length */

                area_pfindex = pageframe_index(
                        (((unsigned long long)mmap->base_addr_high)<<32) +
                                              mmap->base_addr_low);

                area_nframes = pageframe_count(
                        (((unsigned long long)mmap->length_high)<<32) +
                                              mmap->length_low);

                /* area at address 0, or too small */
                if (!area_pfindex || (area_nframes < nframes)) {
                        continue;
                }

                /* possible index */
                pfindex = area_pfindex;

                /* check for intersection with kernel */
                if (!range_order(kernel_pfindex,
                                 kernel_pfindex+kernel_nframes,
                                 pfindex,
                                 pfindex+nframes)) {
                        pfindex = kernel_pfindex+kernel_nframes;
                }

                /* check for intersection with modules */

                while ( !pfoffset &&
                       ((pfindex+nframes) < (area_pfindex+area_nframes)) ) {

                        size_t j;
                        const struct multiboot_module *mod;

                        mod = (const struct multiboot_module*)mb_info->mods_addr;

                        for (j = 0; j < mb_info->mods_count; ++j, ++mod) {
                                unsigned long mod_pfindex;
                                unsigned long mod_nframes;

                                mod_pfindex = pageframe_index(mod->mod_start);
                                mod_nframes = pageframe_count(mod->mod_end -
                                                              mod->mod_start);

                                /* check intersection */
                                if (range_order(mod_pfindex,
                                                mod_pfindex+mod_nframes,
                                                pfindex,
                                                pfindex+nframes)) {
                                        /* no intersection, offset found */
                                        pfoffset = pageframe_offset(pfindex);
                                } else {
                                        pfindex = mod_pfindex+mod_nframes;
                                }
                        }
                }
        }

        return pfoffset;
}

static int
init_physmem(const struct multiboot_header *mb_header,
             const struct multiboot_info *mb_info)
{
        unsigned long physmap, pfcount;

        pfcount = pageframe_count((mb_info->mem_upper+1)<<10);

        physmap = find_unused_area(mb_header,
                                   mb_info,
                                   pfcount>>PAGEFRAME_SHIFT);

        console_printf("found phymap area at %x\n", (unsigned long)physmap);

        /* init memory manager */
        return physmem_init(physmap, pfcount);
}

static int
init_physmap_kernel(const struct multiboot_header *mb_header)
{
        unsigned long pfindex, nframes;

        pfindex = pageframe_index(mb_header->load_addr);
        nframes = pageframe_count(mb_header->bss_end_addr-mb_header->load_addr);

        return physmem_add_area(pfindex, nframes, PHYSMEM_FLAG_RESERVED);
}

static int
init_physmap_areas(const struct multiboot_info *mb_info)
{
        size_t i;
        unsigned long mmap_addr;

        mmap_addr = mb_info->mmap_addr;

        /* add memory areas */

        for (i = 0; i < mb_info->mmap_length;) {
                const struct multiboot_mmap *mmap;
                unsigned long pfindex;
                unsigned long nframes;

                mmap = (const struct multiboot_mmap*)mmap_addr;

                pfindex = pageframe_index(
                        (((unsigned long long)mmap->base_addr_high)<<32) +
                                              mmap->base_addr_low);

                nframes = pageframe_count(
                        (((unsigned long long)mmap->length_high)<<32) +
                                              mmap->length_low);

                physmem_add_area(pfindex,
                                 nframes,
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
                unsigned long pfindex;
                unsigned long nframes;

                pfindex = pageframe_index(mod->mod_start);
                nframes = pageframe_count(mod->mod_end-mod->mod_start);

                physmem_add_area(pfindex, nframes, PHYSMEM_FLAG_RESERVED);
        }

        return 0;
}

#include "page.h"
#include "pte.h"
#include "mmu.h"
#include "tcb.h"
#include "task.h"
#include "string.h"
#include "tid.h"
#include "taskmngr.h"

static int
load_modules(const struct multiboot_info *mb_info)
{
        size_t i;
        const struct multiboot_module *mod;

        mod = (const struct multiboot_module*)mb_info->mods_addr;

        for (i = 0; i < mb_info->mods_count; ++i, ++mod) {

                struct task *tsk = taskmngr_get_current_task();

                console_printf("%s:%x.\n", __FILE__, __LINE__);
                elf_exec(tsk->pd, (const void*)mod->mod_start);
                console_printf("%s:%x.\n", __FILE__, __LINE__);
        }

        return 0;
}

static void
main_invalop_handler(address_type ip)
{
        console_printf("invalid opcode ip=%x.\n", ip);
}

void
multiboot_main(const struct multiboot_header *mb_header,
               const struct multiboot_info *mb_info,
               void *stack)
{
        int err;

        int_enabled();

        console_printf("%s...\n\t%s\n", "OS kernel booting",
                                        "Cool, isn't it?");

        /* init physical memory with lowest 4 MiB reserved for DMA,
           kernel, etc */

        int_enabled();

        if (mb_info->flags&MULTIBOOT_INFO_FLAG_MEM) {
                init_physmem(mb_header, mb_info);
        } else {
                /* FIXME: abort kernel */
                console_printf("no memory information given.\n");
                return;
        }

        physmem_add_area(0, 1024, PHYSMEM_FLAG_RESERVED);

        int_enabled();

        init_physmap_kernel(mb_header);

        int_enabled();

        if (mb_info->flags&MULTIBOOT_INFO_FLAG_MMAP) {
                init_physmap_areas(mb_info);
                init_physmap_modules(mb_info);
        }
        int_enabled();

        /* setup GDT for protected mode */
        gdt_init();
        int_enabled();
        gdt_install();

        int_enabled();

        /* setup IDT for protected mode */
        idt_init();
        int_enabled();
        idt_install();
        int_enabled();

        idt_install_invalid_opcode_handler(main_invalop_handler);

/*        __asm__("hlt\n\t");*/

        /* setup interupt controller */
        pic_install();

        /* setup keyboard */
        if ((err = kbd_init()) < 0) {
                console_perror("kbd_init", -err);
        } else {
                idt_install_irq_handler(1, kbd_irq_handler);
        }

        /* setup PIT for system timer */
        pit_install(0, 20, PIT_MODE_RATEGEN);
        idt_install_irq_handler(0, pit_irq_handler);

        sti();

        /* build initial task and address space */

        idt_install_segfault_handler(virtmem_segfault_handler);
        idt_install_pagefault_handler(virtmem_pagefault_handler);

        if ((err = taskmngr_init()) < 0) {
                console_perror("taskmngr_init", -err);
                return;
        }

        /* test allocation */
        {
                struct task *tsk = taskmngr_get_current_task();

                int *addr = page_address(virtmem_alloc_pages_in_area(
                                        tsk->pd,
                                        2,
                                        g_virtmem_area+VIRTMEM_AREA_USER,
                                        PTE_FLAG_PRESENT|
                                        PTE_FLAG_WRITEABLE));

                if (!addr) {
                        console_printf("alloc error %s %x.\n", __FILE__, __LINE__);
                }

                console_printf("alloced addr=%x.\n", addr);

                memset(addr, 0, 2*PAGE_SIZE);
        }

        {
                struct task *tsk = taskmngr_get_current_task();

                int *addr = page_address(virtmem_alloc_pages_in_area(
                                        tsk->pd,
                                        1023,
                                        g_virtmem_area+VIRTMEM_AREA_USER,
                                        PTE_FLAG_PRESENT|
                                        PTE_FLAG_WRITEABLE));

                if (!addr) {
                        console_printf("alloc error %s %x.\n", __FILE__, __LINE__);
                }

                console_printf("alloced addr=%x.\n", addr);

                memset(addr, 0, 1023*PAGE_SIZE);
        }

        /* setup kernel as thread 0 of kernel task */

        {
                struct task *tsk = taskmngr_get_current_task();

                struct tcb *tcb = task_get_tcb(tsk, 0);

                if ((err = tcb_init(tcb, stack)) < 0) {
                        console_perror("tcb_init", -err);
                }
        }

        /* load init task */

        if (mb_info->flags&MULTIBOOT_INFO_FLAG_MODS) {
                console_printf("%s:%x.\n", __FILE__, __LINE__);
                load_modules(mb_info);
                console_printf("%s:%x.\n", __FILE__, __LINE__);
        }

        return;

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

