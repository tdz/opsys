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

.global tcb_switch_to_ready
.global tcb_switch_to_ready_entry_point
.global tcb_switch_to_ready_return_firsttime

/* int tcb_switch_to_ready(struct tcb *src, struct tcb *dst, int dohalt);
 */
tcb_switch_to_ready:
        pushl %ebp
        movl %esp, %ebp

        cli

        /* save source regs
         */

        /* save %eax first, as it later contains the source tcb address */
        pushl %edx
        movl 8(%ebp), %edx

        addl $16, %edx /* adjust %edx to point to first register in source */
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
        movl $tcb_switch_to_ready_entry_point, 60(%eax)

        /* restore destination regs
         */

        movl 12(%ebp), %eax
        addl $16, %eax

        /* restore page directory */
        movl 52(%eax), %edx
        movl %edx, %cr3

        /* restore base and stack pointer*/
        movl 24(%eax), %ebp
        movl 28(%eax), %esp

        sti

        /* jump to dst %eip */
        jmp * 60(%eax)

tcb_switch_to_ready_entry_point:

        /* any previously saved destination will jump here, so %cr3, %esp,
           and %ebp are already loaded */

        cli

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
        /* %esp and %ebp hasve been restored by src tcb */

        /* general-purpose registers */
        movl 4(%eax), %ebx
        movl 8(%eax), %ecx
        movl 12(%eax), %edx

        sti

        movl $0, %eax
        popl %ebp
        ret

tcb_switch_to_ready_return_firsttime:
        /* remove tcb_switch_to_ready parameters */
        addl $12, %esp;
        jmp isr_handle_irq_return

