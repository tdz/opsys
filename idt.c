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
#include "intrrupt.h"

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

void
timer_handler(unsigned long eip,
              unsigned long cs,
              unsigned long eflags)
{
        extern unsigned long tickcounter;

        __asm__("pusha\n\t");

        ++tickcounter;
/*        eoi(0);*/

        __asm__("popa\n\t");
}

void
keyboard_handler(unsigned long eip,
                 unsigned long cs,
                 unsigned long eflags)
{
        console_printf("keyboard handler\n");
        eoi(1);
}

void
unhandled_irq_handler(unsigned long eip,
                      unsigned long cs,
                      unsigned long eflags)
{
        console_printf("unhandled IRQ eip=%x cs=%x eflags=%x\n", eip,
                                                                 cs,
                                                                 eflags);
}

void
default_handler(void)
{
        console_printf("default handler\n");
}

void
unhandled_interrupt_handler(unsigned long eip,
                            unsigned long cs,
                            unsigned long eflags)
{
        console_printf("unhandled exception eip=%x cs=%x eflags=%x\n", eip,
                                                                       cs,
                                                                       eflags);
}

void
int_segfault(unsigned long err,
             unsigned long eip,
             unsigned long cs,
             unsigned long eflags)
{
        console_printf("segfault cs=%x err=%x eip=%x eflags=%x\n", cs,
                                                                   err,
                                                                   eip,
                                                                   eflags);
}

void
invalid_opcode_handler(unsigned long eip,
                       unsigned long cs,
                       unsigned long eflags)
{
        console_printf("invalid opcode eip=%x cs=%x eflags=%x\n", eip,
                                                                  cs,
                                                                  eflags);
}

void handle_interrupt0(void);
void handle_interrupt1(void);
void handle_interrupt2(void);
void handle_interrupt3(void);
void handle_interrupt4(void);
void handle_interrupt5(void);
void handle_interrupt6(void);
void handle_interrupt7(void);
void handle_interrupt8(void);
void handle_interrupt9(void);
void handle_interrupt10(void);
void handle_interrupt11(void);
void handle_interrupt12(void);
void handle_interrupt13(void);
void handle_interrupt14(void);
void handle_interrupt15(void);
void handle_interrupt16(void);
void handle_interrupt17(void);
void handle_interrupt18(void);
void handle_interrupt19(void);
void handle_interrupt20(void);
void handle_interrupt21(void);
void handle_interrupt22(void);
void handle_interrupt23(void);
void handle_interrupt24(void);
void handle_interrupt25(void);
void handle_interrupt26(void);
void handle_interrupt27(void);
void handle_interrupt28(void);
void handle_interrupt29(void);
void handle_interrupt30(void);
void handle_interrupt31(void);

void handle_irq0(void);
void handle_irq1(void);
void handle_irq2(void);
void handle_irq3(void);
void handle_irq4(void);
void handle_irq5(void);
void handle_irq6(void);
void handle_irq7(void);
void handle_irq8(void);
void handle_irq9(void);
void handle_irq10(void);
void handle_irq11(void);
void handle_irq12(void);
void handle_irq13(void);
void handle_irq14(void);
void handle_irq15(void);

void handle_unknown_interrupt(void);
void handle_invalid_opcode_interrupt(void);

static struct idt_entry g_idt[256];

void
idt_init(void)
{
#define IDT_ENTRY_INIT(index, funcname)                         \
        idt_entry_init(g_idt+(index),                           \
                       (unsigned long)funcname,                 \
                       0x08,                                    \
                       0,                                       \
                       IDT_FLAG_SEGINMEM|IDT_FLAG_32BITINT)

        size_t i;

        for (i = 0; i < sizeof(g_idt)/sizeof(g_idt[0]); ++i) {
                IDT_ENTRY_INIT(i, handle_unknown_interrupt);
        }

        /* System interrupts */

        IDT_ENTRY_INIT(0x00, handle_interrupt0);
        IDT_ENTRY_INIT(0x01, handle_interrupt1);
        IDT_ENTRY_INIT(0x02, handle_interrupt2);
        IDT_ENTRY_INIT(0x03, handle_interrupt3);
        IDT_ENTRY_INIT(0x04, handle_interrupt4);
        IDT_ENTRY_INIT(0x05, handle_interrupt5);
        IDT_ENTRY_INIT(0x06, handle_interrupt6);
        IDT_ENTRY_INIT(0x07, handle_interrupt7);
        IDT_ENTRY_INIT(0x08, handle_interrupt8);
        IDT_ENTRY_INIT(0x09, handle_interrupt9);
        IDT_ENTRY_INIT(0x0a, handle_interrupt10);
        IDT_ENTRY_INIT(0x0b, handle_interrupt11);
        IDT_ENTRY_INIT(0x0c, handle_interrupt12);
        IDT_ENTRY_INIT(0x0d, handle_interrupt13);
        IDT_ENTRY_INIT(0x0e, handle_interrupt14);
        IDT_ENTRY_INIT(0x0f, handle_interrupt15);
        IDT_ENTRY_INIT(0x10, handle_interrupt16);
        IDT_ENTRY_INIT(0x11, handle_interrupt17);
        IDT_ENTRY_INIT(0x12, handle_interrupt18);
        IDT_ENTRY_INIT(0x13, handle_interrupt19);
        IDT_ENTRY_INIT(0x14, handle_interrupt20);
        IDT_ENTRY_INIT(0x15, handle_interrupt21);
        IDT_ENTRY_INIT(0x16, handle_interrupt22);
        IDT_ENTRY_INIT(0x17, handle_interrupt23);
        IDT_ENTRY_INIT(0x18, handle_interrupt24);
        IDT_ENTRY_INIT(0x19, handle_interrupt25);
        IDT_ENTRY_INIT(0x1a, handle_interrupt26);
        IDT_ENTRY_INIT(0x1b, handle_interrupt27);
        IDT_ENTRY_INIT(0x1c, handle_interrupt28);
        IDT_ENTRY_INIT(0x1d, handle_interrupt29);
        IDT_ENTRY_INIT(0x1e, handle_interrupt30);
        IDT_ENTRY_INIT(0x1f, handle_interrupt31);

        /* hardware interrupts */

        IDT_ENTRY_INIT(0x20, handle_irq0);
        IDT_ENTRY_INIT(0x21, handle_irq1);
        IDT_ENTRY_INIT(0x22, handle_irq2);
        IDT_ENTRY_INIT(0x23, handle_irq3);
        IDT_ENTRY_INIT(0x24, handle_irq4);
        IDT_ENTRY_INIT(0x25, handle_irq5);
        IDT_ENTRY_INIT(0x26, handle_irq6);
        IDT_ENTRY_INIT(0x27, handle_irq7);
        IDT_ENTRY_INIT(0x28, handle_irq8);
        IDT_ENTRY_INIT(0x29, handle_irq9);
        IDT_ENTRY_INIT(0x2a, handle_irq10);
        IDT_ENTRY_INIT(0x2b, handle_irq11);
        IDT_ENTRY_INIT(0x2c, handle_irq12);
        IDT_ENTRY_INIT(0x2d, handle_irq13);
        IDT_ENTRY_INIT(0x2e, handle_irq14);
        IDT_ENTRY_INIT(0x2f, handle_irq15);
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

