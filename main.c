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

unsigned long tickcounter = 0;

int
load_modules(struct multiboot_info *mb_info)
{
        if (mb_info->flags&MULTIBOOT_HEADER_FLAG_MODS) {

                size_t i;
                const struct multiboot_module *mod;

                mod = (const struct multiboot_module*)mb_info->mods_addr;

                for (i = 0; i < mb_info->mods_count; ++i, ++mod) {
                        console_printf("found module at %x, size=%x\n",
                                        mod->mod_start,
                                        mod->mod_end-mod->mod_start);
                }
        }

        return 0;
}

void
os_main_from_multiboot(struct multiboot_info *mb_info)
{
        unsigned long esp;

        console_printf("%s...\n\t%s\n", "OS kernel booting",
                                        "Cool, isn't it?");

        /* setup GDT for protected mode */
        gdt_init();
        gdt_install();

        /* setup IDT for protected mode */
        idt_init();
        idt_install();
        idt_install_irq();

        /* setup PIT for system timer */
/*        pit_install(0, 20, PIT_MODE_WAVEGEN);*/

        load_modules(mb_info);

        sti();

        for (;;) {
                __asm__("int $0x20\n\t");
                __asm__("movl %%esp, %0\n\t"
                                : "=r"(esp)
                                :
                                :);

                crt_setpos(12, 40);
                console_printf("%x", esp);
        }
}

