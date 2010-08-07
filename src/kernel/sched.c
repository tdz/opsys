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

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>

#include <cpu.h>
#include <interupt.h>

#include "task.h"

#include "spinlock.h"
#include "list.h"
#include "ipcmsg.h"

#include <tcbregs.h>
#include <tcb.h>
#include "sched.h"

#include "console.h"

/** Current list head in thread list */
static struct list *g_current_thread = NULL;

int
sched_init()
{
        return 0;
}

int
sched_add_thread(struct tcb *tcb)
{
        console_printf("%s:%x adding tcb=%x.\n", __FILE__, __LINE__, tcb);

        list_init(&tcb->sched, &tcb->sched, &tcb->sched, tcb);

        if (!g_current_thread) {
                g_current_thread = &tcb->sched;
        } else {
                list_enque_in_front(g_current_thread, &tcb->sched);
        }

        return 0;
}

struct tcb *
sched_get_current_thread()
{
        return sched_get_thread(g_current_thread);
}

struct tcb *
sched_get_thread(struct list *listhead)
{
        struct tcb *tcb;

        if (!listhead) {
                return NULL;
        }

        tcb = list_data(listhead);

        return tcb;
}

struct tcb *
sched_search_thread(unsigned int taskid, unsigned char tcbid)
{
        struct tcb *tcb;
        struct list *current;

        tcb = NULL;

        if (g_current_thread) {
                current = g_current_thread;

                do {
                        struct tcb *currenttcb = list_data(current);

                        if ((currenttcb->id == tcbid) &&
                            (currenttcb->task->id == taskid)) {
                                tcb = currenttcb;
                        }
                        current = current->next;
                } while (!tcb && (current != g_current_thread));
        }

        return tcb;
}

int
sched_switch_to(struct tcb *next)
{
        int err;
        struct tcb *tcb;

        tcb = list_data(g_current_thread);

        g_current_thread = &next->sched;

        if ((err = tcb_switch(tcb, next)) < 0) {
                goto err_tcb_switch;
        }

        return 0;

err_tcb_switch:
        return err;
}

static struct tcb *
sched_select_thread(void)
{
        struct tcb *tcb;
        struct list *listhead;
        int is_runable;

        if (!g_current_thread) {
                return NULL;
        }

        listhead = g_current_thread;

        do {
                listhead = listhead->next;
                tcb = list_data(listhead);
                is_runable = tcb_is_runnable(tcb);
        } while (!is_runable);

        return tcb;
}

int
sched_switch()
{
        int err;

        do {
                struct tcb *tcb = sched_select_thread();

                if (tcb && tcb_is_runnable(tcb)) {

                        if ((err = sched_switch_to(tcb)) < 0) {
                                goto err_sched_switch_to;
                        }

                        return 0;
                }

                /* no thread ready; wait and search again after wakeup */
                hlt();
        } while (1);

err_sched_switch_to:
        return err;
}

void
sched_irq_handler(unsigned char irqno)
{
        sti();
        sched_switch();
        cli();
}

