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
#include "syscall.h"
#include "idt.h"

enum {
        IDT_FLAG_SEGINMEM = 0x80,
        IDT_FLAG_32BITINT = 0x0e,
        IDT_FLAG_16BITINT = 0x06
};

struct idt_entry
{
        unsigned short base_low;
        unsigned short tss;
        unsigned char  reserved;
        unsigned char  flags;
        unsigned short base_high;
};

static void
idt_entry_init(struct idt_entry *idte, unsigned long  func,
                                       unsigned short tss,
                                       unsigned char  ring,
                                       unsigned char  flags)
{
        idte->base_low = func&0xffff;
        idte->tss = tss;
        idte->reserved = 0;
        idte->flags = flags | ((ring&0x03)<<7);
        idte->base_high = (func>>16)&0xffff;
}

#include "console.h"

static void
default_handler(void)
{
        console_printf("default handler\n");
        return;
}

static void
timer_handler(void)
{
        console_printf("timer handler\n");
        return;
}

static struct idt_entry g_idt[256];

void
idt_init(void)
{
        size_t i;

        for (i = 0; i < sizeof(g_idt)/sizeof(g_idt[0]); ++i) {
                idt_entry_init(g_idt+i, (unsigned long)default_handler,
                                        0x08,
                                        0,
                                        IDT_FLAG_SEGINMEM|
                                        IDT_FLAG_32BITINT);
        }

        idt_entry_init(g_idt+0x20, (unsigned long)timer_handler,
                                   0x08,
                                   0,
                                   IDT_FLAG_SEGINMEM|
                                   IDT_FLAG_32BITINT);
}

struct idt_register
{
        unsigned short limit __attribute__ ((packed));
        unsigned long  base  __attribute__ ((packed));
};

const static struct idt_register idtr __asm__ ("_idtr") = {
        .limit = sizeof(g_idt),
        .base  = (unsigned long)g_idt,
};

void
idt_install()
{
        __asm__ ("lidt (_idtr)\n\t");
}

void
idt_install_irq()
{
        __asm__ (/* out ICW 1 */
                 "mov $0x11, %%al\n\t"
                 "out %%al, $0x20\n\t"
                 "out %%al, $0xa0\n\t"
                 /* out ICW 2 */
                 "mov $0x20, %%al\n\t"
                 "out %%al, $0x21\n\t"
                 "mov $0x28, %%al\n\t"
                 "out %%al, $0xa1\n\t"
                 /* out ICW 3 */
                 "mov $0x04, %%al\n\t"
                 "out %%al, $0x21\n\t"
                 "mov $0x02, %%al\n\t"
                 "out %%al, $0xa1\n\t"
                 /* out ICW 4 */
                 "mov $0x01, %%al\n\t"
                 "out %%al, $0x21\n\t"
                 "out %%al, $0xa1\n\t"
                        :
                        :
                        : "al");
}

