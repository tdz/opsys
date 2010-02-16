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
#include <stddef.h>
#include <sys/types.h>

#include <debug.h>
#include <interupt.h>

#include "alloc.h"
#include "list.h"
#include "ipcmsg.h"

#include "tid.h"
#include "task.h"
#include <tcb.h>

#include "sched.h"
#include "syscall.h"

#include "console.h"

enum {
        TIMEOUT_NOW   = 0,
        TIMEOUT_NEVER = 0x7fffffff
};

#define R0_THREADID(x_)         ((x_)&0xffffffff)

/* asyncronous delivery */
#define R1_SYNC(x_)             ((x_)&0x00000001)

/* sender waits for reply */
#define R1_TIMEOUT(x_)          (((x_)&0xfffffffe)>>1)

static int
r1_has_timeout(unsigned long flags)
{
        return (R1_TIMEOUT(flags) != TIMEOUT_NOW) &&
               (R1_TIMEOUT(flags) != TIMEOUT_NEVER);
}

int
syscall_entry_handler(unsigned long r0,
                      unsigned long r1,
                      unsigned long r2,
                      unsigned long r3)
{
        int enable_int;
        int err;
        struct tcb *snd, *rcv;
/*        struct ipc_msg *msg;
        struct list *ipcin;*/

        enable_int = 0;int_enabled();

        if (enable_int) {
                cli();
        }

        console_printf("%s:%x: r0=%x r1=%x r2=%x r3=%x.\n", __FILE__, __LINE__,
                        r0, r1, r2, r3);

        /* get current thread */

        if (!(snd = sched_get_current_thread())) {
                err = -EAGAIN;
                goto err_sched_get_current_thread;
        }

        console_printf("%s:%x %x:%x.\n", __FILE__, __LINE__,
                        threadid_get_taskid(R0_THREADID(r0)),
                        threadid_get_tcbid(R0_THREADID(r0)));

        /* get receiver thread */

        rcv = sched_search_thread(threadid_get_taskid(R0_THREADID(r0)),
                                  threadid_get_tcbid(R0_THREADID(r0)));
        if (!rcv) {
                err = -EAGAIN;
                goto err_sched_search_thread;
        }

        /* check if rcv is ready to receive */

        if ((R1_TIMEOUT(r1) == TIMEOUT_NOW)
                && (tcb_get_state(rcv) != THREAD_STATE_RECV)) {
                err = -EBUSY;
                goto err_r1_sync;
        }

        /* create IPC message */

        if ((err = ipc_msg_init(&snd->msg, snd, r1, r2, r3)) < 0) {
                goto err_ipc_msg_init;
        }

        if (rcv->ipcin) {
                rcv->ipcin = list_init(&snd->ipcout, rcv->ipcin->prev,
                                                     rcv->ipcin,
                                                    &snd->msg);
        } else {
                rcv->ipcin = list_init(&snd->ipcout, NULL, NULL, &snd->msg);
        }

        /* sender state */

        tcb_set_state(snd, THREAD_STATE_RECV);

        if (r1_has_timeout(r1)) {
                /* TODO: implement timeout */
        }

        /* wake up receiver if necessary, and schedule */

        if (tcb_get_state(rcv) == THREAD_STATE_RECV) {
                tcb_set_state(rcv, THREAD_STATE_READY);
        }

        console_printf("%s:%x cr3=%x pd=%x.\n", __FILE__, __LINE__,
                                snd->cr3, snd->task->pd);

        console_printf("%s:%x current=%x s%%eps=%x r%%eps=%x.\n", __FILE__, __LINE__, snd, snd->esp, rcv->esp);

        sched_switch(1);

        console_printf("%s:%x current=%x.\n", __FILE__, __LINE__, sched_get_current_thread());

/*        __asm__("dohlt: hlt\n\t"
                "jmp dohlt\n\t");*/

        /* sender is always ready when returning here */

        /* before returning, sender should have received a reply */

        if (enable_int) {
                sti();
        }

        console_printf("%s:%x cr3=%x pd=%x.\n", __FILE__, __LINE__,
                                snd->cr3, snd->task->pd);

        return 0;

err_ipc_msg_init:
err_r1_sync:
err_sched_search_thread:
err_sched_get_current_thread:
        if (enable_int) {
                sti();
        }
        return err;
}

