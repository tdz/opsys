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

.global isr_handle_irq_return

isr_drop_interupt:
        iret

isr_handle_invalop:
        cmp $0, invalop_handler
        je isr_handle_invalop_ret
        pushl %eax
        pushl %ecx
        pushl %edx
        pushl 12(%esp)
        movl invalop_handler, %eax
        call *%eax
        addl $4, %esp
        popl %edx
        popl %ecx
        popl %eax
isr_handle_invalop_ret:
        iret

isr_handle_segfault:
        cmp $0, segfault_handler
        je isr_handle_segfault_ret
        pushl %eax
        pushl %ecx
        pushl %edx
        pushl 12(%esp)
        movl segfault_handler, %eax
        call *%eax
        addl $4, %esp
        popl %edx
        popl %ecx
        popl %eax
isr_handle_segfault_ret:
        iret

isr_handle_pagefault:
        cmp $0, pagefault_handler
        je isr_handle_pagefault_ret
        pushl %eax
        pushl %ecx
        pushl %edx
        movl %cr2, %eax
        pushl %eax
        pushl 12(%esp)
        movl pagefault_handler, %eax
        call *%eax
        addl $8, %esp
        popl %edx
        popl %ecx
        popl %eax
isr_handle_pagefault_ret:
        iret

isr_handle_irq:
        /* save registers (according to cdecl convention) */
        pushl %eax
        pushl %ecx
        pushl %edx
        /* compute irq_table[irqno] in %eax */
        movl 12(%esp), %eax
        cmp $16, %eax
        jnl isr_handle_irq_ret
        shll $2, %eax
        addl $irq_table, %eax
        cmp $0, %eax
        je isr_handle_irq_ret
        /* call irq_table[irqno] */
        pushl 12(%esp)
        call *(%eax)
isr_handle_irq_return:
        addl $4, %esp
isr_handle_irq_ret:
        /* signal eoi to PIC */
        pushl 12(%esp)
        call eoi
        addl $4, %esp
        /* restore registers */
        popl %edx
        popl %ecx
        popl %eax
        addl $4, %esp /* remove irqno */
        iret

isr_handle_irq0:
        pushl $0
        jmp isr_handle_irq

isr_handle_irq1:
        pushl $1
        jmp isr_handle_irq

isr_handle_irq2:
        pushl $2
        jmp isr_handle_irq

isr_handle_irq3:
        pushl $3
        jmp isr_handle_irq

isr_handle_irq4:
        pushl $4
        jmp isr_handle_irq

isr_handle_irq5:
        pushl $5
        jmp isr_handle_irq

isr_handle_irq6:
        pushl $6
        jmp isr_handle_irq

isr_handle_irq7:
        pushl $7
        jmp isr_handle_irq

isr_handle_irq8:
        pushl $8
        jmp isr_handle_irq

isr_handle_irq9:
        pushl $9
        jmp isr_handle_irq

isr_handle_irq10:
        pushl $10
        jmp isr_handle_irq

isr_handle_irq11:
        pushl $11
        jmp isr_handle_irq

isr_handle_irq12:
        pushl $12
        jmp isr_handle_irq

isr_handle_irq13:
        pushl $13
        jmp isr_handle_irq

isr_handle_irq14:
        pushl $14
        jmp isr_handle_irq

isr_handle_irq15:
        pushl $15
        jmp isr_handle_irq

isr_handle_syscall:
        /* syscalls use cdecl convention and the application is responsible
           for saving the GP registers. The code below doesn't have to care
           about this. */

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

