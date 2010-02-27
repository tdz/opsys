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

struct tcb_regs
{
        /* general-purpose registers */
        unsigned long eax;
        unsigned long ebx;
        unsigned long ecx;
        unsigned long edx;

        /* index registers */
        unsigned long esi;
        unsigned long edi;
        unsigned long ebp;
        unsigned long esp;

        /* segment registers */
        unsigned short cs;
        unsigned short ds;
        unsigned short ss;
        unsigned short es;
        unsigned short fs;
        unsigned short gs;

        /* state registers */
        unsigned long cr0;
        unsigned long cr2;
        unsigned long cr3;
        unsigned long cr4;
        unsigned long eip;
        unsigned long eflags;
        unsigned long tr; /* task register */

        /* FPU registers */
        unsigned long st[8];

        /* MMX registers */
        unsigned long mm[8];

        /* SSE registers */
        unsigned long xmm[8];
        unsigned long mxcsr;

        /* debug registers */        
        unsigned long dr0;
        unsigned long dr1;
        unsigned long dr2;
        unsigned long dr3;
        unsigned long dr6;
        unsigned long dr7;
};

int
tcb_regs_init(struct tcb_regs *regs, void *stack, void *tlps);

void
tcb_regs_uninit(struct tcb_regs *regs);

void
tcb_regs_init_state(struct tcb_regs *regs);

void
tcb_regs_set_ip(struct tcb_regs *regs, void *ip);

unsigned char *
tcb_regs_get_sp(struct tcb_regs *regs);

void
tcb_regs_set_fp(struct tcb_regs *regs, void *fp);

unsigned char *
tcb_regs_get_fp(struct tcb_regs *regs);

void
tcb_regs_stack_push(struct tcb_regs *regs, size_t nbytes);

void
tcb_regs_stack_pop(struct tcb_regs *regs, size_t nbytes);

int
tcb_regs_switch(struct tcb_regs *src, const struct tcb_regs *dst);

