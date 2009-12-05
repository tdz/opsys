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
#include "physpage.h"
#include "physmem.h"

static int
init_physmem(const struct multiboot_info *mb_info)
{
        size_t i;
        unsigned long mmap_addr;

        /* init memory manager */
        return physmem_init(((mb_info->mem_upper+1)<<10)>>PHYSPAGE_SHIFT);
}

static int
init_physmap(const struct multiboot_info *mb_info)
{
        size_t i;
        unsigned long mmap_addr;

        mmap_addr = mb_info->mmap_addr-4;

        /* add memory areas */

        for (i = 0; i < mb_info->mmap_length; ++i) {
                const struct multiboot_mmap *mmap;
                unsigned long pgoffset;
                unsigned long npages;

                mmap = (const struct multiboot_mmap*)mmap_addr;

                pgoffset = mmap->base_addr_high<<(32-PHYSPAGE_SHIFT) +
                          (mmap->base_addr_low>>PHYSPAGE_SHIFT);

                npages = 1 + mmap->length_high<<(32-PHYSPAGE_SHIFT) +
                            (mmap->length_low>>PHYSPAGE_SHIFT);

                physmem_add_area(pgoffset,
                                 npages,
                                 mmap->type==1 ? PHYSMEM_FLAG_USEABLE
                                               : PHYSMEM_FLAG_RESERVED);

                mmap_addr += mmap->size;                
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
                console_printf("found module at %x, size=%x\n",
                                mod->mod_start,
                                mod->mod_end-mod->mod_start);

                elf_exec((const void*)mod->mod_start);
        }

        return 0;
}

unsigned long tickcounter = 0;

void
multiboot_main(const struct multiboot_info *mb_info)
{
        unsigned long esp;

        console_printf("%s...\n\t%s\n", "OS kernel booting",
                                        "Cool, isn't it?");

/*        if (mb_info->flags&MULTIBOOT_INFO_FLAG_MEM) {
                init_physmem(mb_info);
        }
        if (mb_info->flags&MULTIBOOT_INFO_FLAG_MMAP) {
                init_physmap(mb_info);
        }*/

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

