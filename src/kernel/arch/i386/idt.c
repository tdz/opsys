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
#include "types.h"
#include "syscall.h"
#include "idt.h"
#include "idtentry.h"
#include "interupt.h"
#include "console.h"

/* system interupts
 */

static void (*invalop_handler)(address_type ip);
static void (*segfault_handler)(address_type ip);
static void (*pagefault_handler)(address_type ip, address_type addr);

void
int_handler_invalop(unsigned long eip, unsigned long cs, unsigned long eflags)
{
        __asm__("pusha\n\t");

        if (invalop_handler) {
                invalop_handler(eip);
        }

        __asm__("popa\n\t");
}

void
int_handler_segfault(unsigned long err,
                     unsigned long eip,
                     unsigned long cs,
                     unsigned long eflags)
{
        __asm__("pusha\n\t");

        if (segfault_handler) {
                segfault_handler(eip);
        }

        __asm__("popa\n\t");
}

void
int_handler_pagefault(unsigned long err,
                      unsigned long eip,
                      unsigned long cs,
                      unsigned long eflags)
{
        unsigned long addr;

        __asm__("pusha\n\t"
                "movl %%cr2, %0\n\t"
                        : "=r"(addr));

        if (pagefault_handler) {
                pagefault_handler(eip, addr);
        }

        __asm__("popa\n\t");
}

/* hardware interupts
 */

static void (*irq_table[16])(unsigned char);

void
int_handler_irq(unsigned long irqno,
                unsigned long eip,
                unsigned long cs,
                unsigned long eflags)
{
        __asm__("pusha\n\t");

        size_t irq_table_len = sizeof(irq_table)/sizeof(irq_table[0]);

        if ((irqno < irq_table_len) && irq_table[irqno]) {
                irq_table[irqno](irqno);
        }

        eoi(irqno);

        __asm__("popa\n\t");
}

static struct idt_entry g_idt[256];

void
idt_init()
{
#define IDT_ENTRY_INIT(index, funcname)                                 \
        idt_entry_init(g_idt+(index),                                   \
                       (unsigned long)funcname,                         \
                       0x08,                                            \
                       0,                                               \
                       IDT_ENTRY_FLAG_SEGINMEM|IDT_ENTRY_FLAG_32BITINT)

        extern void isr_drop_interupt(void);
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

        for (i = 0; i < sizeof(g_idt)/sizeof(g_idt[0]); ++i) {
                IDT_ENTRY_INIT(i, isr_drop_interupt);
        }

        /* system interupts */

        IDT_ENTRY_INIT(0x06, isr_handle_invalop);
        IDT_ENTRY_INIT(0x0d, isr_handle_segfault);
        IDT_ENTRY_INIT(0x0e, isr_handle_pagefault);

        /* hardware interupts */

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

        /* syscall interrupt */

        IDT_ENTRY_INIT(0x80, isr_handle_syscall);
}

struct idt_register
{
        unsigned short limit __attribute__ ((packed));
        unsigned long  base  __attribute__ ((packed));
};

void
idt_install()
{
        const static struct idt_register idtr = {
                .limit = sizeof(g_idt),
                .base  = (unsigned long)g_idt,
        };

        __asm__ ("lidt (%0)\n\t"
                        :
                        : "r"(&idtr));
}

int
idt_install_invalid_opcode_handler(void (*hdlr)(address_type))
{
        invalop_handler = hdlr;

        return 0;
}

int
idt_install_segfault_handler(void (*hdlr)(address_type))
{
        segfault_handler = hdlr;

        return 0;
}

int
idt_install_pagefault_handler(void (*hdlr)(address_type, address_type))
{
        pagefault_handler = hdlr;

        return 0;
}

int
idt_install_irq_handler(unsigned char irqno, void (*hdlr)(unsigned char))
{
        if (irqno < sizeof(irq_table)/sizeof(irq_table[0])) {
                irq_table[irqno] = hdlr;
        }

        return 0;
}

