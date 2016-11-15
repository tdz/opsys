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

#include "tcb.h"
#include <arch/i386/page.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include "console.h"
#include "cpu.h"
#include "bitset.h"
#include "pageframe.h"
#include "task.h"
#include "vmem.h"

struct tcb*
tcb_of_sched_list(struct list* l)
{
    return containerof(l, struct tcb, sched);
}

struct tcb*
tcb_of_wait_list(struct list* l)
{
    return containerof(l, struct tcb, wait);
}

int
tcb_init_with_id(struct tcb *tcb,
                 struct task *task, unsigned char id, void *stack)
{
        int err;
        os_index_t pfindex;

        console_printf("tcb id=%x.\n", id);

        if ((err = task_ref(task)) < 0)
        {
                goto err_task_ref;
        }

        if (bitset_isset(task->threadid, id))
        {
                err = -EINVAL;
                goto err_bitset_isset;
        }

        bitset_set(task->threadid, id);

        memset(tcb, 0, sizeof(*tcb));

        tcb->state = THREAD_STATE_ZOMBIE;
        tcb->task = task;
        tcb->stack = stack;
        tcb->id = id;
        tcb->ipcin = NULL;

        list_init(&tcb->wait);
        list_init(&tcb->sched);

        spinlock_init(&tcb->lock);

        pfindex = vmem_lookup_frame(task->as, page_index(task->as->tlps));

        if (pfindex < 0)
        {
                err = pfindex;
                goto err_vmem_lookup_pageframe;
        }

        err = tcb_regs_init(&tcb->regs, stack, pageframe_address(pfindex));

        if (err < 0)
        {
                goto err_tcb_regs_init;
        }

        return 0;

err_tcb_regs_init:
err_vmem_lookup_pageframe:
        spinlock_uninit(&tcb->lock);
        bitset_unset(task->threadid, id);
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

        if (id < 0)
        {
                err = id;
                goto err_bitset_find_unset;
        }

        if ((err = tcb_init_with_id(tcb, task, id, stack)) < 0)
        {
                goto err_tcb_init_with_id;
        }

        return 0;

err_tcb_init_with_id:
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

int
tcb_is_runnable(const struct tcb *tcb)
{
        return (tcb->state == THREAD_STATE_READY);
}

int
tcb_switch(struct tcb *tcb, const struct tcb *dst)
{
        return tcb_regs_switch(&tcb->regs, &dst->regs);
}

int
tcb_set_initial_ready_state(struct tcb *tcb,
                            const void *ip,
                            unsigned char irqno,
                            unsigned long *stack, int nargs, ...)
{
    va_list ap;
    va_start(ap, nargs);
    tcb_regs_init_state(&tcb->regs, ip, irqno, stack, nargs, ap);
    va_end(ap);

    tcb_set_state(tcb, THREAD_STATE_READY);

    return 0;
}
