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
#include <string.h>
#include <sys/types.h>

#include <cpu.h>
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

static enum syscall_op
syscall_get_op(unsigned long flags)
{
        return flags >> 28;
}

void
syscall_entry_handler(unsigned long *tid,
                      unsigned long *flags,
                      unsigned long *msg0,
                      unsigned long *msg1)
{
        static int (* const opfunc[])(struct ipc_msg*, struct tcb*) =
        {
                [SYSCALL_OP_SEND]           = ipc_send,
                [SYSCALL_OP_SEND_AND_WAIT]  = ipc_send_and_wait,
                [SYSCALL_OP_RECV]           = ipc_recv,
                [SYSCALL_OP_REPLY_AND_RECV] = ipc_reply_and_recv
        };

        int err;
        struct tcb *snd, *rcv;
        enum syscall_op op;

        console_printf("%s:%x: tid=%x flags=%x msg0=%x msg1=%x.\n", __FILE__,
                       __LINE__, *tid, *flags, *msg0, *msg1);

        op = syscall_get_op(*flags);

        if (!(op < ARRAY_NELEMS(opfunc)) || !opfunc[op])
        {
                err = -ENOSYS;
                goto err_syscall_op;
        }

        /*
         * get current thread 
         */

        if (!(snd = sched_get_current_thread(cpuid())))
        {
                err = -EAGAIN;
                goto err_sched_get_current_thread;
        }

        /*
         * get receiver thread 
         */

        rcv = sched_search_thread(threadid_get_taskid(R0_THREADID(*tid)),
                                  threadid_get_tcbid(R0_THREADID(*tid)));
        if (!rcv)
        {
                err = -EAGAIN;
                goto err_sched_search_thread;
        }

        /*
         * send message to receiver 
         */

        if ((err = ipc_msg_init(&snd->msg, snd, *flags&~IPC_MSG_FLAGS_RESERVED, *msg0, *msg1)) < 0)
        {
                goto err_ipc_msg_init;
        }

        if ((err = opfunc[op](&snd->msg, rcv)) < 0)
        {
                goto err_opfunc;
        }

        /*
         * sender is always ready when returning here 
         */

        if (ipc_msg_flags_is_errno(&snd->msg))
        {
                err = -snd->msg.msg0;
                goto err_ipc_msg_flags_is_errno;
        }

        /*
         * before returning, sender should have received a reply 
         */

        *tid = threadid_create(snd->msg.snd->task->id, snd->msg.snd->id);
        *flags = snd->msg.flags;
        *msg0 = snd->msg.msg0;
        *msg1 = snd->msg.msg1;

        return;

err_ipc_msg_flags_is_errno:
err_opfunc:
err_ipc_msg_init:
err_sched_search_thread:
err_sched_get_current_thread:
err_syscall_op:
        *flags = IPC_MSG_FLAG_IS_ERRNO;
        *msg0 = err;
}

