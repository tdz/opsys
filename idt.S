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
        push $0
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt1:
        push $1
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt2:
        push $2
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt3:
        push $3
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt4:
        push $4
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt5:
        push $5
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt6:
        push $6
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt7:
        push $7
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt8:
        push $8
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt9:
        push $9
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt10:
        push $10
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt11:
        push $11
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt12:
        push $12
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt13:
        call int_segfault
        iret

handle_interrupt14:
        push $14
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt15:
        push $15
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt16:
        push $16
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt17:
        push $17
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt18:
        push $18
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt19:
        push $19
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt20:
        push $20
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt21:
        push $21
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt22:
        push $22
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt23:
        push $23
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt24:
        push $24
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt25:
        push $25
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt26:
        push $26
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt27:
        push $27
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt28:
        push $28
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt29:
        push $29
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt30:
        push $30
        call interrupt_handler
        add $1, %esp
        iret

handle_interrupt31:
        push $31
        call interrupt_handler
        add $1, %esp
        iret

handle_irq0:
        call timer_handler
        iret

handle_irq1:
        call keyboard_handler
        iret

handle_irq2:
        push $2
        call irq_handler
        add $1, %esp
        iret

handle_irq3:
        push $3
        call irq_handler
        add $1, %esp
        iret

handle_irq4:
        push $4
        call irq_handler
        add $1, %esp
        iret

handle_irq5:
        push $5
        call irq_handler
        add $1, %esp
        iret

handle_irq6:
        push $6
        call irq_handler
        add $1, %esp
        iret

handle_irq7:
        push $7
        call irq_handler
        add $1, %esp
        iret

handle_irq8:
        push $8
        call irq_handler
        add $1, %esp
        iret

handle_irq9:
        push $9
        call irq_handler
        add $1, %esp
        iret

handle_irq10:
        push $10
        call irq_handler
        add $1, %esp
        iret

handle_irq11:
        push $11
        call irq_handler
        add $1, %esp
        iret

handle_irq12:
        push $12
        call irq_handler
        add $1, %esp
        iret

handle_irq13:
        push $13
        call irq_handler
        add $1, %esp
        iret

handle_irq14:
        push $14
        call irq_handler
        add $1, %esp
        iret

handle_irq15:
        push $15
        call irq_handler
        add $1, %esp
        iret

