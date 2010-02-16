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
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#include "cpu.h"

#include "bitset.h"

/* virtual memory */
#include "page.h"
#include "virtmem.h"

#include "task.h"
#include <ipcmsg.h>
#include <list.h>

#include "tcb.h"

#include "console.h"

static int
tcb_set_page_directory(struct tcb *tcb, const struct page_directory *pd)
{
        int err;
        os_index_t pfindex;

        if ((pfindex = virtmem_lookup_pageframe(pd, page_index(pd))) < 0) {
                err = pfindex;
                goto err_virtmem_lookup_pageframe;
        }

        tcb->cr3 = (pfindex<<12) | (tcb->cr3&0xfff);

        return 0;

err_virtmem_lookup_pageframe:
        return err;
}

int
tcb_init_with_id(struct tcb *tcb,
                 struct task *task, unsigned char id, void *stack)
{
        int err;

        console_printf("tcb id=%x.\n", id);

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
        tcb->ipcin = NULL;

        list_init(&tcb->ipcout, NULL, NULL, NULL);

        tcb->esp = (unsigned long)tcb->stack;
        tcb->ebp = tcb->esp;

        if ((err = tcb_set_page_directory(tcb, tcb->task->pd)) < 0) {
                goto err_tcb_set_page_directory;
        }

        return 0;

err_tcb_set_page_directory:
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

unsigned char *
tcb_stack_top(struct tcb *tcb)
{
        return (void*)tcb->esp;
}

void
tcb_stack_push4(struct tcb *tcb, unsigned long value)
{
        unsigned long *stack = (void*)tcb->esp;

        stack[-1] = value;
        tcb->esp -= 4;
}

static int
tcb_switch_to_zombie(struct tcb *tcb, const struct tcb *dst, int dohalt)
{
        return 0;
}

/* implemented in tcb.S */
int
tcb_switch_to_ready(struct tcb *tcb, const struct tcb *dst, int dohalt);

static int
tcb_switch_to_send(struct tcb *tcb, const struct tcb *dst, int dohalt)
{
        return 0;
}

static int
tcb_switch_to_recv(struct tcb *tcb, const struct tcb *dst, int dohalt)
{
        return 0;
}

int
tcb_switch(struct tcb *tcb, const struct tcb *dst, int dohalt)
{
        static int (* const switch_to[])(struct tcb*, const struct tcb*, int) = {
                tcb_switch_to_zombie,
                tcb_switch_to_ready,
                tcb_switch_to_send,
                tcb_switch_to_recv};

/*        console_printf("%s:%x dst=%x dst->state=%x.\n", __FILE__, __LINE__, dst, dst->state);*/

        return switch_to[dst->state](tcb, dst, dohalt | (dst == (void*)0xc0805000));
}

int
tcb_set_initial_ready_state(struct tcb *tcb,
                            const void *ip,
                            unsigned char irqno,
                            int nargs, ...)
{
        extern void tcb_switch_to_ready_entry_point(void);
        extern void tcb_switch_to_ready_return_firsttime(void);

        va_list ap;

        console_printf("%s:%x tcb=%x pd=%x stack=%x.\n", __FILE__, __LINE__, tcb, tcb->cr3, tcb->stack);
        

        /* generate thread execution stack */

        va_start(ap, nargs);

        while (nargs) {
                unsigned long arg = va_arg(ap, unsigned long);

                tcb_stack_push4(tcb, arg);

                --nargs;
        }

        va_end(ap);

        tcb_stack_push4(tcb, 0x1); /* no return ip */

        /* prepare stack as if tcb was scheduled from irq */

        /* stack after irq */
        tcb_stack_push4(tcb, eflags());
        tcb_stack_push4(tcb, cs());
        tcb_stack_push4(tcb, (unsigned long)ip);
        tcb_stack_push4(tcb, irqno);

        tcb_stack_push4(tcb, 0xdeadbeef); /* %eax */
        tcb_stack_push4(tcb, 0xdeadbeef); /* %ecx */
        tcb_stack_push4(tcb, 0xdeadbeef); /* %edx */
        tcb_stack_push4(tcb, irqno); /* pushed by irq handler */

        /* tcb_switch */
        tcb_stack_push4(tcb, 1);
        tcb_stack_push4(tcb, (unsigned long)tcb);
        tcb_stack_push4(tcb, (unsigned long)tcb);
        tcb_stack_push4(tcb, (unsigned long)tcb_switch_to_ready_return_firsttime);
        tcb_stack_push4(tcb, tcb->ebp); /* %ebp */
        tcb->ebp = (unsigned long)tcb_stack_top(tcb);

        tcb->cr0 = cr0();
        tcb->cr2 = cr2();
        tcb->cr4 = cr4();
        tcb->eflags = eflags();

        tcb_set_ip(tcb, tcb_switch_to_ready_entry_point);
        tcb_set_state(tcb, THREAD_STATE_READY);

        console_printf("%s:%x tcb=%x esp=%x ebp=%x.\n", __FILE__, __LINE__, tcb, tcb->esp, tcb->ebp);

        return 0;
}

