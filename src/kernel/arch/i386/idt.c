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

#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#include "syscall.h"
#include "idt.h"
#include "idtentry.h"
#include "interupt.h"

/* system interupts
 */

void (*invalop_handler) (void *ip);
void (*segfault_handler) (void *ip);
void (*pagefault_handler) (void *ip, void *addr, unsigned long);
void (*syscall_handler) (unsigned long *r0,
                         unsigned long *r1,
                         unsigned long *r2, unsigned long *r3);

/* hardware interupts
 */

void (*irq_table[16]) (unsigned char);

static struct idt_entry g_idt[256];

void
idt_init()
{
#define IDT_ENTRY_INIT(index, funcname)                                 \
        idt_entry_init(g_idt+(index),                                   \
                       (unsigned long)funcname,                         \
                       0x08,                                            \
                       0,                                               \
                       IDT_ENTRY_FLAG_SEGINMEM|IDT_ENTRY_FLAG_32BITINT|IDT_ENTRY_FLAG_INTGATE)

        extern void isr_drop_interupt(void);
        extern void isr_handle_debug(void);
        extern void isr_handle_invalop(void);
        extern void isr_handle_segfault(void);
        extern void isr_handle_pagefault(void);
        extern void isr_handle_irq0(void);
        extern void isr_handle_irq1(void);
        extern void isr_handle_irq2(void);
        extern void isr_handle_irq3(void);
        extern void isr_handle_irq4(void);
        extern void isr_handle_irq5(void);
        extern void isr_handle_irq6(void);
        extern void isr_handle_irq7(void);
        extern void isr_handle_irq8(void);
        extern void isr_handle_irq9(void);
        extern void isr_handle_irq10(void);
        extern void isr_handle_irq11(void);
        extern void isr_handle_irq12(void);
        extern void isr_handle_irq13(void);
        extern void isr_handle_irq14(void);
        extern void isr_handle_irq15(void);
        extern void isr_handle_syscall(void);

        size_t i;

        for (i = 0; i < ARRAY_NELEMS(g_idt); ++i)
        {
                IDT_ENTRY_INIT(i, isr_drop_interupt);
        }

        /*
         * system interupts 
         */

        IDT_ENTRY_INIT(0x01, isr_handle_invalop);
        IDT_ENTRY_INIT(0x06, isr_handle_invalop);
        IDT_ENTRY_INIT(0x0d, isr_handle_segfault);
        IDT_ENTRY_INIT(0x0e, isr_handle_pagefault);

        /*
         * hardware interupts 
         */

        IDT_ENTRY_INIT(0x20, isr_handle_irq0);
        IDT_ENTRY_INIT(0x21, isr_handle_irq1);
        IDT_ENTRY_INIT(0x22, isr_handle_irq2);
        IDT_ENTRY_INIT(0x23, isr_handle_irq3);
        IDT_ENTRY_INIT(0x24, isr_handle_irq4);
        IDT_ENTRY_INIT(0x25, isr_handle_irq5);
        IDT_ENTRY_INIT(0x26, isr_handle_irq6);
        IDT_ENTRY_INIT(0x27, isr_handle_irq7);
        IDT_ENTRY_INIT(0x28, isr_handle_irq8);
        IDT_ENTRY_INIT(0x29, isr_handle_irq9);
        IDT_ENTRY_INIT(0x2a, isr_handle_irq10);
        IDT_ENTRY_INIT(0x2b, isr_handle_irq11);
        IDT_ENTRY_INIT(0x2c, isr_handle_irq12);
        IDT_ENTRY_INIT(0x2d, isr_handle_irq13);
        IDT_ENTRY_INIT(0x2e, isr_handle_irq14);
        IDT_ENTRY_INIT(0x2f, isr_handle_irq15);

        /*
         * syscall interrupt 
         */

        IDT_ENTRY_INIT(0x80, isr_handle_syscall);
}

struct idt_register
{
        unsigned short limit __attribute__ ((packed));
        unsigned long base __attribute__ ((packed));
};

void
idt_install()
{
        const static struct idt_register idtr = {
                .limit = sizeof(g_idt),
                .base = (unsigned long)g_idt,
        };

        __asm__("lidt (%0)\n\t"
                :
                :"r"(&idtr));
}

int
idt_install_invalid_opcode_handler(void (*hdlr) (void *))
{
        const int ints = int_enabled();

        if (ints)
        {
                cli();
        }

        invalop_handler = hdlr;

        if (ints)
        {
                sti();
        }

        return 0;
}

int
idt_install_segfault_handler(void (*hdlr) (void *))
{
        const int ints = int_enabled();

        if (ints)
        {
                cli();
        }

        segfault_handler = hdlr;

        if (ints)
        {
                sti();
        }

        return 0;
}

int
idt_install_pagefault_handler(void (*hdlr) (void *, void *, unsigned long))
{
        const int ints = int_enabled();

        if (ints)
        {
                cli();
        }

        pagefault_handler = hdlr;

        if (ints)
        {
                sti();
        }

        return 0;
}

int
idt_install_irq_handler(unsigned char irqno, void (*hdlr) (unsigned char))
{
        if (irqno < ARRAY_NELEMS(irq_table))
        {
                const int ints = int_enabled();

                if (ints)
                {
                        cli();
                }

                irq_table[irqno] = hdlr;

                if (ints)
                {
                        sti();
                }
        }

        return 0;
}

int
idt_install_syscall_handler(void (*hdlr) (unsigned long *,
                                          unsigned long *,
                                          unsigned long *, unsigned long *))
{
        const int ints = int_enabled();

        if (ints)
        {
                cli();
        }

        syscall_handler = hdlr;

        if (ints)
        {
                sti();
        }

        return 0;
}
