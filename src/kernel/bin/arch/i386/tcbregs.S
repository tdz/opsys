/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2010  Thomas Zimmermann <tdz@users.sourceforge.net>
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

.global tcb_regs_switch
.global tcb_regs_switch_entry_point
.global tcb_regs_switch_first_return

/* int tcb_regs_switch(struct tcb_regs *src, const struct tcb_regs *dst);
 */
tcb_regs_switch:
        pushl %ebp
        movl %esp, %ebp

        /* save source regs
         */

        /* save %eax first, as it later contains the source tcb address */
        pushl %edx
        movl 8(%ebp), %edx

        movl %eax, (%edx)
        movl %edx, %eax
        popl %edx

        /* save other general-purpose registers */
        movl %ebx, 4(%eax)
        movl %ecx, 8(%eax)
        movl %edx, 12(%eax)

        /* index registers */
        movl %esi, 16(%eax)
        movl %edi, 20(%eax)
        movl %ebp, 24(%eax)
        movl %esp, 28(%eax)

        /* flags */
        pushf
        popl 64(%eax)

        /* segmentation registers */
        mov %cs, 32(%eax)
        mov %ds, 34(%eax)
        mov %ss, 36(%eax)
        mov %es, 38(%eax)
        mov %fs, 40(%eax)
        mov %gs, 42(%eax)

        /* control registers (need intermediate register) */
        movl %cr0, %edx
        movl %edx, 44(%eax)
        movl %cr2, %edx
        movl %edx, 48(%eax)
        movl %cr3, %edx
        movl %edx, 52(%eax)
        movl %cr4, %edx
        movl %edx, 56(%eax)

        /* save %eip as return address */
        movl $tcb_regs_switch_entry_point, 60(%eax)

        /* restore destination regs
         */

        movl 12(%ebp), %eax

        /* no interrupts between update of address space and stack */
        cli 

        /* restore page directory */
        movl 52(%eax), %edx
        movl %edx, %cr3

        /* restore base and stack pointer */
        movl 24(%eax), %ebp
        movl 28(%eax), %esp

        sti

        /* jump to dst %eip */
        jmp * 60(%eax)

tcb_regs_switch_entry_point:

        /* any previously saved destination will jump here, so %cr3, %esp,
           and %ebp are already loaded */

        /* control registers (need intermediate register) */
        movl 44(%eax), %edx
        movl %edx, %cr0
        movl 48(%eax), %edx
        movl %edx, %cr2
        /* %cr3 has been restored by src tcb */
        movl 56(%eax), %edx
        movl %edx, %cr4

        /* flags */
        pushl 64(%eax)
        popf

        /* index registers */
        movl 16(%eax), %esi
        movl 20(%eax), %edi
        /* %esp and %ebp have been restored by src tcb */

        /* general-purpose registers */
        movl 4(%eax), %ebx
        movl 8(%eax), %ecx
        movl 12(%eax), %edx

        movl $0, %eax
        popl %ebp
        ret

tcb_regs_switch_first_return:
        /* remove tcb_regs_switch parameters */
        sti /* normally enabled by sched_switch_to */
        addl $8, %esp;
        jmp isr_handle_irq_return

