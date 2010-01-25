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

.global tcb_switch_to_starting
.global tcb_switch_to_ready

/* void tcb_save(struct tcb *src);
 */
tcb_save : 
        pushl %ebp
        movl %esp, %ebp

        cli

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

        sti

        popl %ebp
        ret

/* int tcb_switch_to_starting(struct tcb *src, struct tcb *dst);
 */
tcb_switch_to_starting:
        pushl %ebp
        movl %esp, %ebp

        /* save regs */
        pushl 8(%ebp)
        call tcb_save
        addl $4, %esp

        cli

        /* from here on we load destination 12(%ebp) */

        movl 12(%ebp), %eax
        addl $16, %eax

        movl 52(%eax), %edx
        movl %edx, %cr3

        movl 28(%eax), %esp

        sti

        jmp * 60(%eax)

    tcb_switch_to_starting_return_address:

        movl $0, %eax
        pop %ebp
        ret

/* int tcb_switch_to_ready(struct tcb *src, struct tcb *dst);
 */
tcb_switch_to_ready:
        pushl %ebp
        movl %esp, %ebp

        movl $0, %eax
        pop %ebp
        ret

