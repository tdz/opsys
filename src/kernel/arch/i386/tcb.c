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

#include <errno.h>
#include <string.h>
#include <types.h>

#include "cpu.h"
#include "interupt.h"

#include "bitset.h"

/* virtual memory */
#include "page.h"
#include "pte.h"
#include "pagetbl.h"
#include "pde.h"
#include "pagedir.h"
#include "vmemarea.h"
#include "virtmem.h"

#include "task.h"
#include "tcb.h"

static int
tcb_set_page_directory(struct tcb *tcb, const struct page_directory *pd)
{
        os_index_t pfindex;

        pfindex = virtmem_lookup_pageframe(pd, page_index(pd));

        tcb->cr3 = (pfindex<<12) | (tcb->cr3&0xfff);

        return 0;
}

int
tcb_init_with_id(struct tcb *tcb,
                 struct task *task, unsigned char id, void *stack)
{
        int err;

        if ((err = task_ref(task)) < 0) {
                goto err_task_ref;
        }

        if (bitset_isset(task->threadid, id)) {
                err = -EINVAL;
                goto err_bitset_isset;
        }

        bitset_set(task->threadid, id);

        memset(tcb, 0, sizeof(*tcb));

        tcb->state = THREAD_STATE_ZOMBIE;
        tcb->task = task;
        tcb->stack = stack;
        tcb->id = id;

        tcb->esp = (unsigned long)tcb->stack;

        tcb_set_page_directory(tcb, tcb->task->pd);

        return 0;

err_bitset_isset:
        task_unref(task);
err_task_ref:
        return err;
}

int
tcb_init(struct tcb *tcb, struct task *task, void *stack)
{
        int err;
        ssize_t id;

        id = bitset_find_unset(task->threadid, sizeof(task->threadid));

        if (id < 0) {
                err = id;
                goto err_bitset_find_unset;
        }

        return tcb_init_with_id(tcb, task, id, stack);

err_bitset_find_unset:
        return err;
}

void
tcb_uninit(struct tcb *tcb)
{
        task_unref(tcb->task);
}

void
tcb_set_state(struct tcb *tcb, enum thread_state state)
{
        tcb->state = state;
}

enum thread_state
tcb_get_state(const struct tcb *tcb)
{
        return tcb->state;
}

void
tcb_set_ip(struct tcb *tcb, void *ip)
{
        tcb->eip = (unsigned long)ip;
}

int
tcb_is_runnable(const struct tcb *tcb)
{
        return (tcb->state == THREAD_STATE_READY);
}

static int
tcb_switch_to_zombie(struct tcb *tcb, const struct tcb *dst)
{
        return 0;
}

static int
tcb_switch_to_blocked(struct tcb *tcb, const struct tcb *dst)
{
        return 0;
}

/* implemented in tcb.S */
int
tcb_switch_to_ready(struct tcb *tcb, const struct tcb *dst);

int
tcb_switch(struct tcb *tcb, const struct tcb *dst)
{
        static int (* const switch_to[])(struct tcb*, const struct tcb*) = {
                tcb_switch_to_zombie,
                tcb_switch_to_ready,
                tcb_switch_to_blocked};

        return switch_to[dst->state](tcb, dst);
}

int
tcb_set_initial_ready_state(struct tcb *tcb,
                            const void *ip,
                            unsigned char irqno)
{
        extern void tcb_switch_to_ready_entry_point(void);
        extern void tcb_switch_to_ready_return_firsttime(void);

        unsigned long *stack;

        /* prepare stack as if tcb was scheduled from irq */

        stack = tcb->stack;

        /* stack after irq */
        stack[-1] = eflags();
        stack[-2] = cs();
        stack[-3] = (unsigned long)ip;
        stack[-4] = irqno;

        stack[-5] = 0; /* %eax */
        stack[-6] = 0; /* %ecx */
        stack[-7] = 0; /* %edx */
        stack[-8] = irqno; /* pushed by irq handler */

        /* tcb_switch */
        stack[-9] = (unsigned long)tcb;
        stack[-10] = (unsigned long)tcb;
        stack[-11] = (unsigned long)tcb_switch_to_ready_return_firsttime;
        stack[-12] = (unsigned long)(stack-11);

        tcb->cr0 = cr0();
        tcb->cr2 = cr2();
        tcb->cr4 = cr4();
        tcb->eflags = eflags();
        tcb->esp = (unsigned long)(stack-12);
        tcb->ebp = (unsigned long)(stack-11);

        tcb_set_ip(tcb, tcb_switch_to_ready_entry_point);
        tcb_set_state(tcb, THREAD_STATE_READY);

        return 0;
}

