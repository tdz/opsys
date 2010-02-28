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
#include "spinlock.h"

#include "alloc.h"
#include "list.h"
#include "ipcmsg.h"
#include "ipc.h"

#include "tid.h"
#include "task.h"
#include <tcbregs.h>
#include <tcb.h>

#include "sched.h"
#include "syscall.h"

#include "console.h"

#define R0_THREADID(x_)         ((threadid_type)((x_)&0xffffffff))

void
syscall_entry_handler(unsigned long *r0,
                      unsigned long *r1,
                      unsigned long *r2,
                      unsigned long *r3)
{
        int enable_int;
        int err;
        struct tcb *snd, *rcv;

        enable_int = int_enabled();

        if (enable_int) {
                cli();
        }

        console_printf("%s:%x: r0=%x r1=%x r2=%x r3=%x.\n", __FILE__, __LINE__,
                        *r0, *r1, *r2, *r3);

        /* get current thread */

        if (!(snd = sched_get_current_thread())) {
                err = -EAGAIN;
                goto err_sched_get_current_thread;
        }

        /* get receiver thread */

        rcv = sched_search_thread(threadid_get_taskid(R0_THREADID(*r0)),
                                  threadid_get_tcbid(R0_THREADID(*r0)));
        if (!rcv) {
                err = -EAGAIN;
                goto err_sched_search_thread;
        }

        /* send message to receiver */

        if ((err = ipc_msg_init(&snd->msg, snd, *r1, *r2, *r3)) < 0) {
                goto err_ipc_msg_init;
        }

        if ((err = ipc_send_and_wait(&snd->msg, rcv)) < 0) {
                goto err_ipc_send_and_wait;
        }

        /* sender is always ready when returning here */

        if (ipc_msg_flags_is_errno(&snd->msg)) {
                err = -snd->msg.msg0;
                goto err_ipc_msg_flags_is_errno;
        }

        /* before returning, sender should have received a reply */

        if (enable_int) {
                sti();
        }

        *r0 = threadid_create(snd->msg.snd->task->id, snd->msg.snd->id);
        *r1 = snd->msg.flags;
        *r2 = snd->msg.msg0;
        *r3 = snd->msg.msg1;

        return;

err_ipc_msg_flags_is_errno:
err_ipc_send_and_wait:
err_ipc_msg_init:
err_sched_search_thread:
err_sched_get_current_thread:
        if (enable_int) {
                sti();
        }

        *r1 = IPC_MSG_FLAG_IS_ERRNO;
        *r2 = err;
}

