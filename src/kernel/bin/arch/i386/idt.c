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

#include "idt.h"
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include "idtentry.h"
#include "interupt.h"

/* Interupt entry points in idt.S */
extern void idt_ignore_interupt(void);
extern void idt_handle_debug(void);
extern void idt_handle_invalid_opcode(void);
extern void idt_handle_segmentation_fault(void);
extern void idt_handle_page_fault(void);
extern void idt_handle_irq0(void);
extern void idt_handle_irq1(void);
extern void idt_handle_irq2(void);
extern void idt_handle_irq3(void);
extern void idt_handle_irq4(void);
extern void idt_handle_irq5(void);
extern void idt_handle_irq6(void);
extern void idt_handle_irq7(void);
extern void idt_handle_irq8(void);
extern void idt_handle_irq9(void);
extern void idt_handle_irq10(void);
extern void idt_handle_irq11(void);
extern void idt_handle_irq12(void);
extern void idt_handle_irq13(void);
extern void idt_handle_irq14(void);
extern void idt_handle_irq15(void);
extern void idt_handle_syscall(void);

struct idt_register {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry g_idt[256];

const static struct idt_register idtr = {
    .limit = (uint16_t)sizeof(g_idt),
    .base = (uint32_t)g_idt,
};

void
init_idt()
{
#define IDT_ENTRY_INIT(index, funcname)         \
    idt_entry_init(g_idt+(index),               \
                   (unsigned long)funcname,     \
                   0x08,                        \
                   0,                           \
                   IDT_ENTRY_FLAG_SEGINMEM|     \
                   IDT_ENTRY_FLAG_32BITINT|     \
                   IDT_ENTRY_FLAG_INTGATE)

    for (size_t i = 0; i < ARRAY_NELEMS(g_idt); ++i) {
        IDT_ENTRY_INIT(i, idt_ignore_interupt);
    }

    /* CPU exceptions */
    IDT_ENTRY_INIT(0x01, idt_handle_debug);
    IDT_ENTRY_INIT(0x06, idt_handle_invalid_opcode);
    IDT_ENTRY_INIT(0x0d, idt_handle_segmentation_fault);
    IDT_ENTRY_INIT(0x0e, idt_handle_page_fault);

    /* hardware interupts */
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0x0, idt_handle_irq0);
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0x1, idt_handle_irq1);
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0x2, idt_handle_irq2);
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0x3, idt_handle_irq3);
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0x4, idt_handle_irq4);
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0x5, idt_handle_irq5);
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0x6, idt_handle_irq6);
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0x7, idt_handle_irq7);
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0x8, idt_handle_irq8);
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0x9, idt_handle_irq9);
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0xa, idt_handle_irq10);
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0xb, idt_handle_irq11);
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0xc, idt_handle_irq12);
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0xd, idt_handle_irq13);
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0xe, idt_handle_irq14);
    IDT_ENTRY_INIT(IDT_IRQ_OFFSET + 0xf, idt_handle_irq15);

    /* syscall interrupt */
    IDT_ENTRY_INIT(0x80, idt_handle_syscall);

#undef IDT_ENTRY_INIT

    /* load the IDT into the IDTR */
    __asm__("lidt (%0)\n\t"
            :
            :"r"(&idtr));
}
