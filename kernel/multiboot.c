/*
 *  oskernel - A small experimental operating-system kernel
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

#include "types.h"
#include "multiboot.h"
#include "console.h"
#include "gdt.h"
#include "idt.h"
#include "intrrupt.h"
#include "pit.h"
#include "pageframe.h"
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
                 unsigned long nframes)
{
        size_t i;
        unsigned long mmap_addr;
        unsigned long kernel_pfindex, kernel_nframes;
        unsigned long pfoffset;

        kernel_pfindex = pageframe_index(mb_header->load_addr);
        kernel_nframes = pageframe_count(mb_header->bss_end_addr -
                                         mb_header->load_addr);

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
        unsigned long physmap, nframes;

        nframes = pageframe_count((mb_info->mem_upper+1)<<10);
        physmap = find_unused_area(mb_header, mb_info, nframes>>PAGEFRAME_SHIFT);

        console_printf("found phymap area at %x\n", (unsigned long)physmap);

        /* init memory manager */
        return physmem_init(physmap, nframes);
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

/*        console_printf("mmap_length=%x\n", mb_info->mmap_length);*/

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

/*                console_printf("%s:%x %x %x %x\n",
                                __FILE__,
                                __LINE__,
                                (pgoffset<<PHYSPAGE_SHIFT),
                                (npages<<PHYSPAGE_SHIFT),
                                mmap->type);*/

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

/*                console_printf("%s:%x %x %x\n",
                                __FILE__,
                                __LINE__,
                                (pgoffset<<PHYSPAGE_SHIFT),
                                (npages<<PHYSPAGE_SHIFT));*/

                physmem_add_area(pfindex, nframes, PHYSMEM_FLAG_RESERVED);
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

#include "page.h"
#include "pte.h"
#include "pagetbl.h"
#include "pde.h"
#include "pagedir.h"
#include "mmu.h"
#include "virtmem.h"
#include "tcb.h"
#include "task.h"

#include "string.h"

static int
build_init_task(void)
{
        int err;
        static struct page_directory init_pd;
        struct page_directory *pd;
        struct task *task;
        struct tcb *tcb;

        pd = &init_pd;

        /* create virtual address space */

        if ((err = page_directory_init(pd)) < 0) {
                goto err_page_directory_init;
        }

        /* build kernel area */

        if ((err = virtmem_install(pd)) < 0) {
                goto err_virtmem_install;
        }

        /* enable paging */

        console_printf("enabling paging\n");
        mmu_load(((unsigned long)pd->entry)&(~0xfff));
        mmu_enable_paging();
        console_printf("paging enabled.\n");

        /* test allocation */
        {
                int *addr = page_address(virtmem_alloc_pages_in_area(
                                        pd,
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
                int *addr = page_address(virtmem_alloc_pages_in_area(
                                        pd,
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

        return 0;


        /* create task 0
         */

        if ( !(task = task_lookup(0)) ) {
                err = -1;
                goto err_task_lookup;
        }

        /* allocate memory for task */
        /* FIXME: do this in virtual memory */
        if (!physmem_alloc_frames_at(pageframe_index((unsigned long)task),
                                     pageframe_count(sizeof(*task)))) {
                err = -1;
                goto err_task_alloc;
        }

        if ((err = task_init(task)) < 0) {
                goto err_task_init;
        }

        task->pd = pd;

        /* create thread (0:0)
         */

        tcb = task_get_tcb(task, 0);

        if (!tcb) {
                err = -1;
                goto err_task_get_thread;
        }

        if ((err = tcb_set_page_directory(tcb, pd)) < 0) {
                goto err_tcb_set_page_directory;
        }

        /* save registers in TCB 0 */

        return 0;

err_tcb_set_page_directory:
err_task_get_thread:
        task_uninit(task);
err_task_init:
        physmem_unref_frames(pageframe_index((unsigned long)task),
                             pageframe_count(sizeof(*task)));
err_task_alloc:
err_task_lookup:
err_virtmem_install:
        page_directory_uninit(pd);
err_page_directory_init:
        physmem_unref_frames(pageframe_index((unsigned long)pd),
                             pageframe_count(sizeof(*pd)));
err_page_directory_alloc:
        return err;
}

unsigned long tickcounter = 0;

void
multiboot_main(const struct multiboot_header *mb_header,
               const struct multiboot_info *mb_info)
{
        int err;

        console_printf("%s...\n\t%s\n", "OS kernel booting",
                                        "Cool, isn't it?");

        if (mb_info->flags&MULTIBOOT_INFO_FLAG_MEM) {
                init_physmem(mb_header, mb_info);
        } else {
                /* FIXME: abort kernel */
        }

        /* Lowest 1 MiB reserved for DMA */
        physmem_add_area(0, 1024, PHYSMEM_FLAG_RESERVED);

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

        sti();

        /* setup PIT for system timer */
/*        pit_install(0, 20, PIT_MODE_RATEGEN);*/

        if (mb_info->flags&MULTIBOOT_INFO_FLAG_MODS) {
                load_modules(mb_info);
        }

        err = build_init_task();

        console_printf("\nbuild_init_task err=%x\n", err);

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

