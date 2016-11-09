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

/**
 * \file sched.c
 * \author Thomas Zimmermann <tdz@users.sourceforge.net>
 * \date 2010
 *
 * The thread scheduler supports simple priorities for threads. The idle
 * thread always runs with a priority of zero, which is the lowest available.
 *
 * The algorithm for selecting the next thread
 *
 *  - first determines the highest priority class with runnable threads, and
 *  - schedules the threads in round-robin order with fixed time slices.
 *
 * \todo Current many scheduler function receive the current CPU's index as
 *       parameter. This can lead to problems when the thread is migrated
 *       between two depended calls. A possible solution is to never migrate
 *       threads that in a system call. (What about kernel threads?) Otherwise
 *       pin the thread to a specific CPU during the critical phase.
 */

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#include <cpu.h>
#include <interupt.h>

#include "assert.h"

#include "task.h"

#include "spinlock.h"
#include "list.h"
#include "ipcmsg.h"

#include <tcbregs.h>
#include <tcb.h>
#include "sched.h"

#include "console.h"

/**
 * \brief current list head in thread list, one per priority class
 * \internal
 */
static struct list *g_thread[SCHED_NPRIOS];

static struct list *g_current_thread[SCHED_NCPUS];

/**
 * \brief init scheduler
 * \param cpu the CPU for which the scheduler gets initialized
 * \param[in] idle the initial idle thread
 * \return 0 on success, or a negative error code otherwise
 *
 * This initializes the scheduler on the given CPU. The passed thread is the
 * initial kernel thread on the CPU and also serves as the CPU's idle thread.
 * It is added to the thread list automatically.
 */
int
sched_init(unsigned int cpu, struct tcb *idle)
{
        assert(cpu < ARRAY_NELEMS(g_current_thread));
        assert(idle);

        g_current_thread[cpu] = &idle->sched;

        return sched_add_thread(idle, 0);
}

/**
 * \brief add a thread to the scheduler
 * \param[in] tcb the new thread
 * \param prio the priority class
 * \return 0 on success, or a negative error code otherwise
 */
int
sched_add_thread(struct tcb *tcb, prio_class_type prio)
{
        console_printf("%s:%x adding tcb=%x, prio=%x.\n", __FILE__, __LINE__, tcb, prio);

        list_init(&tcb->sched, &tcb->sched, &tcb->sched, tcb);

        if (!g_thread[prio])
        {
                g_thread[prio] = &tcb->sched;
        }
        else
        {
                list_enque_in_front(g_thread[prio], &tcb->sched);
        }

        return 0;
}

/**
 * \brief return the thread that is currently scheduled on the CPU
 * \param cpu the CPU on which the thread is running
 * \return the currently scheduled thread's tcb structure
 */
struct tcb *
sched_get_current_thread(unsigned int cpu)
{
        assert(cpu < ARRAY_NELEMS(g_current_thread));

        return sched_get_thread(g_current_thread[cpu]);
}

/**
 * \brief return the thread at the beginning of the list
 * \param[in] listhead a list of threads
 * \return the first thread in the list, or NULL otherwise
 */
struct tcb *
sched_get_thread(struct list *listhead)
{
        struct tcb *tcb;

        if (!listhead)
        {
                return NULL;
        }

        tcb = list_data(listhead);

        return tcb;
}

/**
 * \brief search for a thread with the task and thread id
 * \param taskid a task id
 * \param tcbid a thread id
 * \return the found thread, or NULL otherwise
 */
struct tcb *
sched_search_thread(unsigned int taskid, unsigned char tcbid)
{
        struct tcb *tcb;
        struct list **cur_prio;
        struct list **end_prio;

        tcb = NULL;

        cur_prio = g_thread;
        end_prio = cur_prio + ARRAY_NELEMS(g_thread);

        while (cur_prio < end_prio)
        {
                if (*cur_prio)
                {
                        struct list *current = *cur_prio;

                        do
                        {
                                struct tcb *currenttcb = list_data(current);

                                if ((currenttcb->id == tcbid) &&
                                    (currenttcb->task->id == taskid))
                                {
                                        tcb = currenttcb;
                                }
                                current = current->next;
                        } while (!tcb && (current != *cur_prio));
                }

                ++cur_prio;
        }

        return tcb;
}

/**
 * \brief switch to a specific thread
 * \param cpu the CPU on which the thread is running
 * \param[in] next the tcb of the destination thread
 * \return 0 on success, or a negative error code otherwise
 *
 * \attention The destination thread has to be in runnable state.
 */
int
sched_switch_to(unsigned int cpu, struct tcb *next)
{
        int err;
        struct tcb *tcb;

        assert(cpu < ARRAY_NELEMS(g_current_thread));
        assert(next && tcb_is_runnable(next));

        tcb = list_data(g_current_thread[cpu]);

        /*if (next != tcb)*/
        {
                g_current_thread[cpu] = &next->sched;

                if ((err = tcb_switch(tcb, next)) < 0)
                {
                        goto err_tcb_switch;
                }
        }

        return 0;

err_tcb_switch:
        return err;
}

/**
 * \brief select a runnable thread
 * \return the tcb of a runnable thread, or NULL otherwise
 * \internal
 */
static struct tcb *
sched_select_thread(void)
{
        struct list **end_prio;
        struct list **cur_prio;

        end_prio = g_thread;
        cur_prio = end_prio + ARRAY_NELEMS(g_thread);

        while (cur_prio > end_prio)
        {
                --cur_prio;

                if (*cur_prio)
                {
                        struct list *listhead = *cur_prio;

                        do
                        {
                                listhead = listhead->next;
                                struct tcb *tcb = list_data(listhead);

                                if (tcb_is_runnable(tcb))
                                {
                                        /* found runnable thread with highest prio */
                                        return tcb;
                                }
                        } while (listhead != *cur_prio);
                }
        }

        /* there should always be an idle thread runnable */
        assert_never_reach();

        return NULL;
}

/**
 * \brief switch to any thread the is runnable
 * \param cpu the CPU on which the thread is running
 * \return 0 on success, or a negative error code otherwise
 */
int
sched_switch(unsigned int cpu)
{
        struct tcb *tcb;
        int err;

        /* at least the idle thread should always be runnable */
        tcb = sched_select_thread();

        if ((err = sched_switch_to(cpu, tcb)) < 0)
        {
                goto err_sched_switch_to;
        }

        return 0;

err_sched_switch_to:
        return err;
}

/**
 * \brief handle schedule interrupt
 * \param irqno the interrupt that triggered the call
 *
 * This function is the high-level entry point for the thread-schedule
 * interrupt. It triggers switches to other runnable threads.
 */
void
sched_irq_handler(unsigned char irqno)
{
        sched_switch(cpuid());
}
