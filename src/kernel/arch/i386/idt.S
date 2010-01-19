/*
 *  opsys - A small, experimental operating system
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

.global handle_unknown_interupt

.global handle_interupt0
.global handle_interupt1
.global handle_interupt2
.global handle_interupt3
.global handle_interupt4
.global handle_interupt5
.global handle_interupt6
.global handle_interupt7
.global handle_interupt8
.global handle_interupt9
.global handle_interupt10
.global handle_interupt11
.global handle_interupt12
.global handle_interupt13
.global handle_interupt14
.global handle_interupt15
.global handle_interupt16
.global handle_interupt17
.global handle_interupt18
.global handle_interupt19
.global handle_interupt20
.global handle_interupt21
.global handle_interupt22
.global handle_interupt23
.global handle_interupt24
.global handle_interupt25
.global handle_interupt26
.global handle_interupt27
.global handle_interupt28
.global handle_interupt29
.global handle_interupt30
.global handle_interupt31

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

.global handle_interupt128

handle_unknown_interupt:
        call default_handler
        iret

handle_interupt0:
        call unhandled_interupt_handler
        iret

handle_interupt1:
        call unhandled_interupt_handler
        iret

handle_interupt2:
        call unhandled_interupt_handler
        iret

handle_interupt3:
        call unhandled_interupt_handler
        iret

handle_interupt4:
        call unhandled_interupt_handler
        iret

handle_interupt5:
        call unhandled_interupt_handler
        iret

handle_interupt6:
        call invalid_opcode_handler
        iret

handle_interupt7:
        call unhandled_interupt_handler
        iret

handle_interupt8:
        call unhandled_interupt_handler
        iret

handle_interupt9:
        call unhandled_interupt_handler
        iret

handle_interupt10:
        call unhandled_interupt_handler
        iret

handle_interupt11:
        call unhandled_interupt_handler
        iret

handle_interupt12:
        call unhandled_interupt_handler
        iret

handle_interupt13:
        call int_segfault
        iret

handle_interupt14:
        call page_fault_handler
        hlt
        iret

handle_interupt15:
        call unhandled_interupt_handler
        iret

handle_interupt16:
        call unhandled_interupt_handler
        iret

handle_interupt17:
        call unhandled_interupt_handler
        iret

handle_interupt18:
        call unhandled_interupt_handler
        iret

handle_interupt19:
        call unhandled_interupt_handler
        iret

handle_interupt20:
        call unhandled_interupt_handler
        iret

handle_interupt21:
        call unhandled_interupt_handler
        iret

handle_interupt22:
        call unhandled_interupt_handler
        iret

handle_interupt23:
        call unhandled_interupt_handler
        iret

handle_interupt24:
        call unhandled_interupt_handler
        iret

handle_interupt25:
        call unhandled_interupt_handler
        iret

handle_interupt26:
        call unhandled_interupt_handler
        iret

handle_interupt27:
        call unhandled_interupt_handler
        iret

handle_interupt28:
        call unhandled_interupt_handler
        iret

handle_interupt29:
        call unhandled_interupt_handler
        iret

handle_interupt30:
        call unhandled_interupt_handler
        iret

handle_interupt31:
        call unhandled_interupt_handler
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

handle_interupt128:
        /* read sysno from stack */
        movl %esp, %eax
        addl $12, %eax
        movl (%eax), %eax
        /* compare sysno to length of entry table */
        cmp entry_table_length, %eax
        jnl handle_interrupt128_err_sysno
        /* call system-call handler */
        movl $4, %ecx
        mull %ecx
        movl $entry_table, %edx
        add %eax, %edx
        call *(%edx)
        iret
handle_interrupt128_err_sysno:
        /* invalid sysno, return -ENOSYS */
        movl $81, %eax
        neg %eax
        iret

