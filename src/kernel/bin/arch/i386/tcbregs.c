/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2010  Thomas Zimmermann
 *  Copyright (C) 2016  Thomas Zimmermann
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

#include "tcbregs.h"
#include <string.h>
#include "cpu.h"
#include "pageframe.h"

static int
tcb_regs_set_tlps(struct tcb_regs *regs, void *tlps)
{
        os_index_t pfindex = pageframe_index(tlps);

        regs->cr3 = (pfindex << 12) | (regs->cr3 & 0xfff);

        return 0;
}

int
tcb_regs_init(struct tcb_regs *regs, void *stack, void *tlps)
{
        int err;

        memset(regs, 0, sizeof(*regs));

        if ((err = tcb_regs_set_tlps(regs, tlps)) < 0)
        {
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

static void*
stack_push4(struct tcb_regs* regs, void* stack, unsigned long value)
{
	/* The stack variable is in current address space, while the
	 * stack pointer in %esp is in TCB's address space! Both refer
	 * to the same physical memory loction. */

	tcb_regs_stack_push(regs, sizeof(value));
	unsigned long *top = ((unsigned long*)stack) - 1;
    top[0] = value;

	return top;
}

void
tcb_regs_init_state(struct tcb_regs* regs,
                    const void *ip,
                    unsigned char irqno,
					void* stack,
                    unsigned long nargs, va_list ap)
{
    extern void tcb_regs_switch_entry_point(void);
    extern void tcb_regs_switch_first_return(void);

    for (; nargs; --nargs) {
        unsigned long arg = va_arg(ap, unsigned long);
        stack = stack_push4(regs, stack, arg);
    }

    stack = stack_push4(regs, stack, 0ul);     	/* no return ip */

    /* We prepare the stack as if the thread was scheduled from
     * an IRQ handler. To the scheduler, the new thread will look
     * like a thread that has been preempted before.
     */

    /* stack after irq */
    stack = stack_push4(regs, stack, eflags());
    stack = stack_push4(regs, stack, cs());
    stack = stack_push4(regs, stack, (unsigned long)ip);
    stack = stack_push4(regs, stack, irqno);

    stack = stack_push4(regs, stack, 0ul);     	/* %eax */
    stack = stack_push4(regs, stack, 0ul);     	/* %ecx */
    stack = stack_push4(regs, stack, 0ul);     	/* %edx */
    stack = stack_push4(regs, stack, irqno);	/* pushed by irq handler */

    /* tcb_regs_switch */
    stack = stack_push4(regs, stack, (unsigned long)regs);
    stack = stack_push4(regs, stack, (unsigned long)regs);
    stack = stack_push4(regs, stack, (unsigned long)tcb_regs_switch_first_return);
    stack = stack_push4(regs, stack, regs->ebp);

    regs->ebp = regs->esp;	/* set framepointer to current stack pointer */

    regs->cr0 = cr0();
    regs->cr2 = cr2();
    regs->cr4 = cr4();
    regs->eflags = eflags();

    /* The entry point for the new thread is the same as
     * for threads that have been preempted. */
    regs->eip = (unsigned long)tcb_regs_switch_entry_point;
}

void
tcb_regs_set_ip(struct tcb_regs *regs, void *ip)
{
        regs->eip = (unsigned long)ip;
}

unsigned char *
tcb_regs_get_sp(struct tcb_regs *regs)
{
        return (void *)regs->esp;
}

void
tcb_regs_set_fp(struct tcb_regs *regs, void *fp)
{
        regs->ebp = (unsigned long)fp;
}

unsigned char *
tcb_regs_get_fp(struct tcb_regs *regs)
{
        return (void *)regs->ebp;
}

uintptr_t
tcb_regs_stack_push(struct tcb_regs *regs, size_t nbytes)
{
    return regs->esp -= nbytes;
}

uintptr_t
tcb_regs_stack_pop(struct tcb_regs *regs, size_t nbytes)
{
    return regs->esp += nbytes;
}
