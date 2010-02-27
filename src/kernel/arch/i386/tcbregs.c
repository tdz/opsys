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

#include <string.h>
#include <sys/types.h>

#include "cpu.h"

/* physical memory */
#include "pageframe.h"

#include "tcbregs.h"

static int
tcb_regs_set_tlps(struct tcb_regs *regs, void *tlps)
{
        os_index_t pfindex = pageframe_index(tlps);

        regs->cr3 = (pfindex<<12) | (regs->cr3&0xfff);

        return 0;
}

int
tcb_regs_init(struct tcb_regs *regs, void *stack, void *tlps)
{
        int err;

        memset(regs, 0, sizeof(*regs));

        if ((err = tcb_regs_set_tlps(regs, tlps)) < 0) {
                goto err_tcb_regs_set_tlps;
        }

        regs->esp = (unsigned long)stack;
        regs->ebp = regs->esp;

        return 0;

err_tcb_regs_set_tlps:
        return err;
}

void
tcb_regs_uninit(struct tcb_regs *regs)
{
        return;
}

void
tcb_regs_init_state(struct tcb_regs *regs)
{
        regs->cr0 = cr0();
        regs->cr2 = cr2();
        regs->cr4 = cr4();
        regs->eflags = eflags();
}

void
tcb_regs_set_ip(struct tcb_regs *regs, void *ip)
{
        regs->eip = (unsigned long)ip;
}

unsigned char *
tcb_regs_get_sp(struct tcb_regs *regs)
{
        return (void*)regs->esp;
}

void
tcb_regs_set_fp(struct tcb_regs *regs, void *fp)
{
        regs->ebp = (unsigned long)fp;
}

unsigned char *
tcb_regs_get_fp(struct tcb_regs *regs)
{
        return (void*)regs->ebp;
}

void
tcb_regs_stack_push(struct tcb_regs *regs, size_t nbytes)
{
        regs->esp -= nbytes;
}

void
tcb_regs_stack_pop(struct tcb_regs *regs, size_t nbytes)
{
        regs->esp += nbytes;
}

