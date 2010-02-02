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
#include <sys/types.h>

#include "alloc.h"
#include "list.h"

#include "tid.h"
#include "task.h"
#include <tcb.h>

#include "sched.h"
#include "syscall.h"
#include "ipcmsg.h"

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
        int err;
        struct tcb *snd, *rcv;
        struct ipc_msg *msg;
        struct list *ipcin;

        console_printf("%s:%x: r0=%x r1=%x r2=%x r3=%x.\n", __FILE__, __LINE__,
                        r0, r1, r2, r3);

        /* get current thread */

        if (!(snd = sched_get_current_thread())) {
                err = -EAGAIN;
                goto err_sched_get_current_thread;
        }

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

        if (!(msg = kmalloc(sizeof(*msg)))) {
                err = -ENOMEM;
                goto err_kmalloc_msg;
        }

        if ((err = ipc_msg_init(msg, snd, r1, r2, r3)) < 0) {
                goto err_ipc_msg_init;
        }

        if (!(ipcin = list_prepend(rcv->ipcin, msg))) {
                err = -ENOMEM;
                goto err_list_prepend;
        }

        rcv->ipcin = ipcin;

        /* sender state */

        tcb_set_state(snd, THREAD_STATE_RECV);

        if (r1_has_timeout(r1)) {
                /* TODO: implement timeout */
        }

        /* wake up receiver if necessary, and schedule */

        if (tcb_get_state(rcv) == THREAD_STATE_RECV) {
                tcb_set_state(rcv, THREAD_STATE_READY);
        }

        sched_switch();

        /* before returning, sender should have received a reply */

        if (tcb_get_state(snd) == THREAD_STATE_RECV) {
                tcb_set_state(snd, THREAD_STATE_READY);
        }

        return 0;

err_list_prepend:
err_ipc_msg_init:
        kfree(msg);
err_kmalloc_msg:
err_r1_sync:
err_sched_search_thread:
err_sched_get_current_thread:
        return err;
}

