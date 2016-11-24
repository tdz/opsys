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

.text

.global isr_drop_interupt
.global isr_handle_debug
.global isr_handle_invalop
.global isr_handle_segfault
.global isr_handle_pagefault
.global isr_handle_irqN
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

isr_handle_debug:
        hlt
        jmp isr_handle_debug

isr_handle_invalop:
        pushl %eax
        pushl %ecx
        pushl %edx
        pushl 12(%esp)
        call platform_handle_invalid_opcode
        addl $4, %esp
        popl %edx
        popl %ecx
        popl %eax
        iret

isr_handle_segfault:
        pushl %eax
        pushl %ecx
        pushl %edx
        pushl 16(%esp)
        call platform_handle_segmentation_fault
        addl $4, %esp
        popl %edx
        popl %ecx
        popl %eax
        iret

isr_handle_pagefault:
        pushl %eax
        pushl %ecx
        pushl %edx
        pushl 12(%esp)
        movl %cr2, %eax
        pushl %eax
        pushl 24(%esp)
        call platform_handle_page_fault
        addl $12, %esp
        popl %edx
        popl %ecx
        popl %eax
        iret

isr_handle_irqN:
        /* save registers (according to cdecl convention) */
        pushl %eax
        pushl %ecx
        pushl %edx
        /* call platform IRQ handler */
        pushl 12(%esp)
        call platform_handle_irq
        jmp isr_handle_irq_return

/* Used in tcb_regs_switch_first_return() for setting up a new
 * thread's initial stack. We make the stack look as if the thread
 * had been scheduled from an interupt. When switching to the new
 * thread for the first time, we jump here, clean up the interupt
 * stack and return to normal thread initialization. */
isr_handle_irq_return:
        addl $4, %esp
        /* signal eoi to PIC */
        pushl 12(%esp)
        call platform_eoi
        addl $4, %esp
        /* restore registers */
        popl %edx
        popl %ecx
        popl %eax
        addl $4, %esp /* remove irqno */
        iret

isr_handle_irq0:
        pushl $0
        jmp isr_handle_irqN

isr_handle_irq1:
        pushl $1
        jmp isr_handle_irqN

isr_handle_irq2:
        pushl $2
        jmp isr_handle_irqN

isr_handle_irq3:
        pushl $3
        jmp isr_handle_irqN

isr_handle_irq4:
        pushl $4
        jmp isr_handle_irqN

isr_handle_irq5:
        pushl $5
        jmp isr_handle_irqN

isr_handle_irq6:
        pushl $6
        jmp isr_handle_irqN

isr_handle_irq7:
        pushl $7
        jmp isr_handle_irqN

isr_handle_irq8:
        pushl $8
        jmp isr_handle_irqN

isr_handle_irq9:
        pushl $9
        jmp isr_handle_irqN

isr_handle_irq10:
        pushl $10
        jmp isr_handle_irqN

isr_handle_irq11:
        pushl $11
        jmp isr_handle_irqN

isr_handle_irq12:
        pushl $12
        jmp isr_handle_irqN

isr_handle_irq13:
        pushl $13
        jmp isr_handle_irqN

isr_handle_irq14:
        pushl $14
        jmp isr_handle_irqN

isr_handle_irq15:
        pushl $15
        jmp isr_handle_irqN

isr_handle_syscall:
        pushl %eax
        pushl %ebx
        pushl %ecx
        pushl %edx
        movl %esp, %eax /* pointer r3 */
        pushl %eax
        addl $4, %eax /* pointer r2 */
        pushl %eax
        addl $4, %eax /* pointer r1 */
        pushl %eax
        addl $4, %eax /* pointer r0 */
        pushl %eax
        call platform_handle_syscall
        addl $16, %esp /* remove pointers from stack */
        popl %edx
        popl %ecx
        popl %ebx
        popl %eax
        iret
