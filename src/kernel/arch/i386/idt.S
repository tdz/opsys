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

.text

.global isr_drop_interupt
.global isr_handle_invalop
.global isr_handle_segfault
.global isr_handle_pagefault
.global isr_handle_irq0
.global isr_handle_irq1
.global isr_handle_irq2
.global isr_handle_irq3
.global isr_handle_irq4
.global isr_handle_irq5
.global isr_handle_irq6
.global isr_handle_irq7
.global isr_handle_irq8
.global isr_handle_irq9
.global isr_handle_irq10
.global isr_handle_irq11
.global isr_handle_irq12
.global isr_handle_irq13
.global isr_handle_irq14
.global isr_handle_irq15
.global isr_handle_syscall

isr_drop_interupt:
        iret

isr_handle_invalop:
        call int_handler_invalop
        iret

isr_handle_segfault:
        call int_handler_segfault
        iret

isr_handle_pagefault:
        call int_handler_pagefault
        hlt
        iret

isr_handle_irq0:
        pushl $0
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_irq1:
        pushl $1
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_irq2:
        pushl $2
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_irq3:
        pushl $3
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_irq4:
        pushl $4
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_irq5:
        pushl $5
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_irq6:
        pushl $6
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_irq7:
        pushl $7
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_irq8:
        pushl $8
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_irq9:
        pushl $9
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_irq10:
        pushl $10
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_irq11:
        pushl $11
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_irq12:
        pushl $12
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_irq13:
        pushl $13
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_irq14:
        pushl $14
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_irq15:
        pushl $15
        call int_handler_irq
        addl $4, %esp
        iret

isr_handle_syscall:
        /* read sysno from stack */
        movl 12(%esp), %eax
        /* compare sysno to length of entry table */
        cmp entry_table_length, %eax
        jnl isr_handle_syscall_err_sysno
        /* call system-call handler */
        shl $2, %eax /* offset = sysno*4 = sysno<<2 */
        addl $entry_table, %eax
        call *(%eax)
        iret
isr_handle_syscall_err_sysno:
        /* invalid sysno, return -ENOSYS */
        movl $-81, %eax
        iret

