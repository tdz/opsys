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

#include <types.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

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
#include "kbd.h"
#include "page.h"
#include "pte.h"
#include "mmu.h"
#include "task.h"
#include "taskhlp.h"
#include "tid.h"
#include "tcb.h"
#include "tcbhlp.h"
#include "loader.h"
#include "sched.h"
#include "multiboot.h"

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

                /* area is not useable */
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
multiboot_init_physmem(const struct multiboot_header *mb_header,
                       const struct multiboot_info *mb_info)
{
        int err;
        unsigned long pfindex, pfcount;

        if (!(mb_info->flags&MULTIBOOT_INFO_FLAG_MEM)) {
                err = -EINVAL;
                goto err_multiboot_info_flag_mem;
        }

        pfcount = pageframe_span(0, (mb_info->mem_upper+1024)<<10);

        pfindex = find_unused_area(mb_header,
                                   mb_info,
                                   pfcount>>PAGEFRAME_SHIFT);

        console_printf("found physmap area at %x\n", (unsigned long)pfindex);

        return physmem_init(pfindex, pfcount);

err_multiboot_info_flag_mem:
        return err;
}

static int
multiboot_init_physmem_kernel(const struct multiboot_header *mb_header)
{
        return physmem_set_flags(pageframe_index(mb_header->load_addr),
                                 pageframe_count(mb_header->bss_end_addr-
                                                 mb_header->load_addr),
                                 PHYSMEM_FLAG_RESERVED);
}

static int
multiboot_init_physmem_mmap(const struct multiboot_info *mb_info)
{
        size_t i;
        unsigned long mmap_addr;

        if (!(mb_info->flags&MULTIBOOT_INFO_FLAG_MMAP)) {
                return 0;
        }

        mmap_addr = mb_info->mmap_addr;

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

                physmem_set_flags(pfindex,
                                  nframes,
                                  mmap->type==1 ? PHYSMEM_FLAG_USEABLE
                                                : PHYSMEM_FLAG_RESERVED);

                mmap_addr += mmap->size+4;
                i += mmap->size+4;
        }

        return 0;
}

static int
multiboot_init_physmem_module(const struct multiboot_module *mod)
{
        return physmem_set_flags(pageframe_index(mod->mod_start),
                                 pageframe_count(mod->mod_end-mod->mod_start),
                                 PHYSMEM_FLAG_RESERVED);
}

static int
multiboot_init_physmem_modules(const struct multiboot_info *mb_info)
{
        int err;
        const struct multiboot_module *mod, *modend;

        if (!(mb_info->flags&MULTIBOOT_INFO_FLAG_MODS)) {
                return 0;
        }

        err = 0;
        mod = (const struct multiboot_module*)mb_info->mods_addr;
        modend = mod+mb_info->mods_count;

        while ((mod < modend)
                && ((err = multiboot_init_physmem_module(mod)) < 0)) {
                ++mod;
        }

        return err;
}

static int
multiboot_load_modules(struct tcb *tcb, const struct multiboot_info *mb_info)
{
        int err;
        const struct multiboot_module *mod;
        size_t i;

        if (!(mb_info->flags&MULTIBOOT_INFO_FLAG_MODS)) {
                return 0;
        }

        mod = (const struct multiboot_module*)mb_info->mods_addr;

        for (i = 0; i < mb_info->mods_count; ++i, ++mod) {

                if (mod->string) {
                        console_printf("loading module '\%s'\n", mod->string);
                } else {
                        console_printf("loading module %x\n", i);
                }

                if ((err = loader_exec(tcb, (void*)mod->mod_start)) < 0) {
                        console_perror("loader_exec", -err);
                }
        }

        return 0;
}

static void
main_invalop_handler(void *ip)
{
        console_printf("invalid opcode ip=%x.\n", (unsigned long)ip);
}

void
multiboot_main(const struct multiboot_header *mb_header,
               const struct multiboot_info *mb_info,
               void *stack)
{
        int err;
        struct task *tsk;
        struct tcb *tcb;

        console_printf("opsys booting...\n");

        /* init physical memory with lowest 4 MiB reserved for DMA,
           kernel, etc
         */

        if ((err = multiboot_init_physmem(mb_header, mb_info)) < 0) {
                console_perror("multiboot_init_physmem", -err);
                return;
        }

        physmem_set_flags(0, 1024, PHYSMEM_FLAG_RESERVED);

        if ((err = multiboot_init_physmem_kernel(mb_header)) < 0) {
                console_perror("multiboot_init_physmem_kernel", -err);
                return;
        }
        if ((err = multiboot_init_physmem_mmap(mb_info)) < 0) {
                console_perror("multiboot_init_physmem_mmap", -err);
                return;
        }
        if ((err = multiboot_init_physmem_modules(mb_info)) < 0) {
                console_perror("multiboot_init_physmem_modules", -err);
                return;
        }

        /* setup GDT for protected mode */
        gdt_init();
        gdt_install();

        /* setup IDT for protected mode */
        idt_init();
        idt_install();

        idt_install_invalid_opcode_handler(main_invalop_handler);

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

        /* build initial task and address space
         */

        idt_install_segfault_handler(virtmem_segfault_handler);
        idt_install_pagefault_handler(virtmem_pagefault_handler);

        if ((err = task_helper_allocate_kernel_task(&tsk)) < 0) {
                console_perror("task_helper_init_kernel_task", -err);
                return;
        }

        /* setup current execution as thread 0 of kernel task
         */

        if ((err = tcb_helper_allocate_tcb(tsk, stack, &tcb)) < 0) {
                console_perror("tcb_helper_allocate_tcb", -err);
                return;
        }

        /* load modules as ELF binaries
         */

        if ((err = multiboot_load_modules(tcb, mb_info)) < 0) {
                console_perror("multiboot_load_modules", -err);
                return;
        }

        /* setup scheduler with kernel thread 0
         */

        if ((err = sched_init()) < 0) {
                console_perror("sched_init", -err);
                return;
        }

        if ((err = sched_add_thread(tcb)) < 0) {
                console_perror("sched_add_thread", -err);
                return;
        }

        sched_switch();
}

/* dead code */

int
test_alloc(void)
{
        struct tcb *tcb;
        int *addr;

        tcb = sched_get_current_thread();

        addr = page_address(virtmem_alloc_pages_in_area(
                            tcb->task->pd,
                            2,
                            g_virtmem_area+VIRTMEM_AREA_USER,
                            PTE_FLAG_PRESENT|
                            PTE_FLAG_WRITEABLE));
        if (!addr) {
                console_printf("alloc error %s %x.\n", __FILE__, __LINE__);
        }

        console_printf("alloced addr=%x.\n", addr);

        memset(addr, 0, 2*PAGE_SIZE);

        tcb = sched_get_current_thread();

        addr = page_address(virtmem_alloc_pages_in_area(
                            tcb->task->pd,
                            1023,
                            g_virtmem_area+VIRTMEM_AREA_USER,
                            PTE_FLAG_PRESENT|
                            PTE_FLAG_WRITEABLE));
        if (!addr) {
                console_printf("alloc error %s %x.\n", __FILE__, __LINE__);
        }

        console_printf("alloced addr=%x.\n", addr);

        memset(addr, 0, 1023*PAGE_SIZE);

        return 0;
}

