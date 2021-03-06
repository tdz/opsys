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

#include "sched.h"
#include <stddef.h>
#include <string.h>
#include "assert.h"
#include "console.h"
#include "cpu.h"
#include "task.h"
#include "tcb.h"
#include "timer.h"

/**
 * \brief current list head in thread list, one per priority class
 * \internal
 */
static struct list g_thread[SCHED_NPRIOS];

/**
 * \brief TCB of the currently scheduled thread on each CPU
 */
static struct tcb* g_current_thread[SCHED_NCPUS];

static timeout_t
sched_timeout(void)
{
    return 1000 / SCHED_FREQ;
}

/**
 * This function is the high-level entry point for the thread-schedule
 * interrupt. It triggers switches to other runnable threads.
 */
static timeout_t
alarm_handler(struct alarm* alarm)
{
    sched_switch(cpuid());

    return sched_timeout();
}

static struct alarm g_alarm;

/**
 * \brief init scheduler
 * \param[in] idle the initial idle thread
 * \return 0 on success, or a negative error code otherwise
 *
 * This initializes the scheduler. The passed thread is the idle
 * thread. It is added to the thread list automatically.
 */
int
sched_init(struct tcb* idle)
{
    assert(idle);

    for (size_t i = 0; i < ARRAY_NELEMS(g_current_thread); ++i) {
        g_current_thread[i] = idle;
    }

    for (size_t i = 0; i < ARRAY_NELEMS(g_thread); ++i) {
        list_init_head(g_thread + i);
    }

    alarm_init(&g_alarm, alarm_handler);

    int res = timer_add_alarm(&g_alarm, sched_timeout());
    if (res < 0) {
        goto err_timer_add_alarm;
    }

    res = sched_add_thread(idle, 0);
    if (res < 0) {
        goto err_sched_add_thread;
    }

    return 0;

err_sched_add_thread:
    timer_remove_alarm(&g_alarm);
err_timer_add_alarm:
    for (size_t i = ARRAY_NELEMS(g_current_thread); i;) {
        --i;
        g_current_thread[i] = NULL;
    }
    return res;
}

/**
 * \brief add a thread to the scheduler
 * \param[in] tcb the new thread
 * \param prio the priority class
 * \return 0 on success, or a negative error code otherwise
 */
int
sched_add_thread(struct tcb* tcb, prio_class_type prio)
{
    console_printf("%s:%x adding tcb=%x, prio=%x.\n", __FILE__, __LINE__, tcb, prio);

    list_enqueue_back(g_thread + prio, &tcb->sched);
    tcb->prio = prio;

    return 0;
}

/**
 * \brief return the thread that is currently scheduled on the CPU
 * \param cpu the CPU on which the thread is running
 * \return the currently scheduled thread's tcb structure
 */
struct tcb*
sched_get_current_thread(unsigned int cpu)
{
    assert(cpu < ARRAY_NELEMS(g_current_thread));

    return g_current_thread[cpu];
}

/**
 * \brief search for a thread with the task and thread id
 * \param taskid a task id
 * \param tcbid a thread id
 * \return the found thread, or NULL otherwise
 */
struct tcb*
sched_search_thread(unsigned int taskid, unsigned char tcbid)
{
    const struct list* cur_prio = g_thread;
    const struct list* end_prio = cur_prio + ARRAY_NELEMS(g_thread);

    while (cur_prio < end_prio) {
        struct list* listhead = list_begin(cur_prio);

        while (listhead != list_end(cur_prio)) {
            struct tcb* tcb = tcb_of_sched_list(listhead);

            if ((tcb->id == tcbid) && (tcb->task->id == taskid)) {
                return tcb;
            }
            listhead = list_next(listhead);
        }

        ++cur_prio;
    }

    return NULL;
}

static void
move_thread_to_back(struct tcb* tcb)
{
    list_dequeue(&tcb->sched);
    list_enqueue_back(g_thread + tcb->prio, &tcb->sched);
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
sched_switch_to(unsigned int cpu, struct tcb* next)
{
    assert(cpu < ARRAY_NELEMS(g_current_thread));
    assert(next && tcb_is_runnable(next));

    /* We move the new thread to the end of the scheduler's
     * ready list, so we don't select it over and over again. */
    move_thread_to_back(next);

    struct tcb* self = g_current_thread[cpu];

    if (next == self) {
		return 0; /* nothing to do if we're switching to the same thread */
	}

    /* Calling tcb_switch() means scheduling another thread, We set
     * the new thread as the current one _before_ switching, or it
     * won't know it's own TCB structure otherwise. */
	g_current_thread[cpu] = next;

	int res = tcb_switch(self, next);
    if (res < 0) {
        goto err_tcb_switch;
	}

    return 0;

err_tcb_switch:
    /* tcb_switch() failed, so we set ourselfes as the current
     * thread. */
	g_current_thread[cpu] = self;
    return res;
}

/**
 * \brief select a runnable thread
 * \return the tcb of a runnable thread, or NULL otherwise
 * \internal
 */
static struct tcb*
sched_select_thread(void)
{
    const struct list* min_prio = g_thread;
    const struct list* cur_prio = min_prio + ARRAY_NELEMS(g_thread);

    while (cur_prio > min_prio) {

        --cur_prio;

        struct list* listhead = list_begin(cur_prio);

        while (listhead != list_end(cur_prio)) {
                struct tcb* tcb = tcb_of_sched_list(listhead);

                if (tcb_is_runnable(tcb)) {
                    /* found runnable thread with highest prio */
                    return tcb;
                }
                listhead = list_next(listhead);
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
    /* at least the idle thread should always be runnable */
    struct tcb* tcb = sched_select_thread();

    int res = sched_switch_to(cpu, tcb);
    if (res < 0) {
        return res;
    }

    return 0;
}
