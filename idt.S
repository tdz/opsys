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

.text

.global handle_unknown_interrupt

.global handle_interrupt0
.global handle_interrupt1
.global handle_interrupt2
.global handle_interrupt3
.global handle_interrupt4
.global handle_interrupt5
.global handle_interrupt6
.global handle_interrupt7
.global handle_interrupt8
.global handle_interrupt9
.global handle_interrupt10
.global handle_interrupt11
.global handle_interrupt12
.global handle_interrupt13
.global handle_interrupt14
.global handle_interrupt15
.global handle_interrupt16
.global handle_interrupt17
.global handle_interrupt18
.global handle_interrupt19
.global handle_interrupt20
.global handle_interrupt21
.global handle_interrupt22
.global handle_interrupt23
.global handle_interrupt24
.global handle_interrupt25
.global handle_interrupt26
.global handle_interrupt27
.global handle_interrupt28
.global handle_interrupt29
.global handle_interrupt30
.global handle_interrupt31

.global handle_irq0
.global handle_irq1
.global handle_irq2
.global handle_irq3
.global handle_irq4
.global handle_irq5
.global handle_irq6
.global handle_irq7
.global handle_irq8
.global handle_irq9
.global handle_irq10
.global handle_irq11
.global handle_irq12
.global handle_irq13
.global handle_irq14
.global handle_irq15

handle_unknown_interrupt:
        call default_handler
        iret

handle_interrupt0:
        call unhandled_interrupt_handler
        iret

handle_interrupt1:
        call unhandled_interrupt_handler
        iret

handle_interrupt2:
        call unhandled_interrupt_handler
        iret

handle_interrupt3:
        call unhandled_interrupt_handler
        iret

handle_interrupt4:
        call unhandled_interrupt_handler
        iret

handle_interrupt5:
        call unhandled_interrupt_handler
        iret

handle_interrupt6:
        call invalid_opcode_handler
        iret

handle_interrupt7:
        call unhandled_interrupt_handler
        iret

handle_interrupt8:
        call unhandled_interrupt_handler
        iret

handle_interrupt9:
        call unhandled_interrupt_handler
        iret

handle_interrupt10:
        call unhandled_interrupt_handler
        iret

handle_interrupt11:
        call unhandled_interrupt_handler
        iret

handle_interrupt12:
        call unhandled_interrupt_handler
        iret

handle_interrupt13:
        call int_segfault
        iret

handle_interrupt14:
        call unhandled_interrupt_handler
        iret

handle_interrupt15:
        call unhandled_interrupt_handler
        iret

handle_interrupt16:
        call unhandled_interrupt_handler
        iret

handle_interrupt17:
        call unhandled_interrupt_handler
        iret

handle_interrupt18:
        call unhandled_interrupt_handler
        iret

handle_interrupt19:
        call unhandled_interrupt_handler
        iret

handle_interrupt20:
        call unhandled_interrupt_handler
        iret

handle_interrupt21:
        call unhandled_interrupt_handler
        iret

handle_interrupt22:
        call unhandled_interrupt_handler
        iret

handle_interrupt23:
        call unhandled_interrupt_handler
        iret

handle_interrupt24:
        call unhandled_interrupt_handler
        iret

handle_interrupt25:
        call unhandled_interrupt_handler
        iret

handle_interrupt26:
        call unhandled_interrupt_handler
        iret

handle_interrupt27:
        call unhandled_interrupt_handler
        iret

handle_interrupt28:
        call unhandled_interrupt_handler
        iret

handle_interrupt29:
        call unhandled_interrupt_handler
        iret

handle_interrupt30:
        call unhandled_interrupt_handler
        iret

handle_interrupt31:
        call unhandled_interrupt_handler
        iret

handle_irq0:
/*        call timer_handler*/
        pushl %eax
        mov $0x20, %al
        outb %al, $0x20
/*        popl %eax*/
        addl $4, %esp
        addl $1, tickcounter
        iret

handle_irq1:
        call keyboard_handler
        iret

handle_irq2:
        call unhandled_irq_handler
        iret

handle_irq3:
        call unhandled_irq_handler
        iret

handle_irq4:
        call unhandled_irq_handler
        iret

handle_irq5:
        call unhandled_irq_handler
        iret

handle_irq6:
        call unhandled_irq_handler
        iret

handle_irq7:
        call unhandled_irq_handler
        iret

handle_irq8:
        call unhandled_irq_handler
        iret

handle_irq9:
        call unhandled_irq_handler
        iret

handle_irq10:
        call unhandled_irq_handler
        iret

handle_irq11:
        call unhandled_irq_handler
        iret

handle_irq12:
        call unhandled_irq_handler
        iret

handle_irq13:
        call unhandled_irq_handler
        iret

handle_irq14:
        call unhandled_irq_handler
        iret

handle_irq15:
        call unhandled_irq_handler
        iret

