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
#include <string.h>
#include <sys/types.h>

#include <cpu.h>
#include <interupt.h>

#include "list.h"

#include "ipcmsg.h"
#include "ipc.h"
#include "spinlock.h"

#include <tcbregs.h>
#include <tcb.h>

#include <task.h>

#include <semaphore.h>
#include <vmem.h>

#include "sched.h"

int
ipc_send(struct ipc_msg *msg, struct tcb *rcv)
{
        return -ENOSYS;
}

int
ipc_send_and_wait(struct ipc_msg *msg, struct tcb *rcv)
{
        int err;

        /*
         * check if rcv is ready to receive 
         */

        if ((ipc_msg_flags_get_timeout(msg) == IPC_TIMEOUT_NOW)
            && (tcb_get_state(rcv) != THREAD_STATE_RECV))
        {
                err = -EBUSY;
                goto err_ipc_msg_flags_get_timeout;
        }

        /*
         * enqueue message 
         */
        /*
         * TODO: enqueue at end, not at beginning; to prevent walk over list,
         * first element could have prev pointer set to end of list 
         */

        spinlock_lock(&rcv->lock, (unsigned long)sched_get_current_thread(cpuid()));

        if (rcv->ipcin)
        {
                rcv->ipcin = list_init(&msg->snd->ipcout, rcv->ipcin->prev,
                                       rcv->ipcin, &msg->snd->msg);
        }
        else
        {
                rcv->ipcin = list_init(&msg->snd->ipcout,
                                       NULL, NULL, &msg->snd->msg);
        }

        spinlock_unlock(&rcv->lock);

        /*
         * sender state 
         */

        spinlock_lock(&msg->snd->lock,
                      (unsigned long)sched_get_current_thread(cpuid()));
        tcb_set_state(msg->snd, THREAD_STATE_RECV);
        spinlock_unlock(&msg->snd->lock);

        if (ipc_msg_flags_has_timeout_value(msg))
        {
                /*
                 * TODO: implement timeout 
                 */
        }

        /*
         * wake up receiver if necessary 
         */

        spinlock_lock(&rcv->lock, (unsigned long)sched_get_current_thread(cpuid()));

        if (tcb_get_state(rcv) == THREAD_STATE_RECV)
        {
                tcb_set_state(rcv, THREAD_STATE_READY);
        }

        spinlock_unlock(&rcv->lock);

        sched_switch(cpuid());

        return 0;

err_ipc_msg_flags_get_timeout:
        return err;
}

int
ipc_reply(struct ipc_msg *msg, struct tcb *rcv)
{
        int err;

        spinlock_lock(&rcv->lock, (unsigned long)sched_get_current_thread(cpuid()));

        if (tcb_get_state(rcv) != THREAD_STATE_RECV)
        {
                err = -EBUSY;
                goto err_tcb_get_state;
        }

        tcb_set_state(rcv, THREAD_STATE_READY);

        spinlock_unlock(&rcv->lock);

        return 0;

err_tcb_get_state:
        spinlock_unlock(&rcv->lock);
        return err;
}
#include <pde.h>
#include <console.h>
int
ipc_recv(struct ipc_msg *msg, struct tcb *rcv)
{
        int err;
        const struct ipc_msg *msgin;

        spinlock_lock(&rcv->lock, (unsigned long)sched_get_current_thread(cpuid()));

        if (!rcv->ipcin)
        {
                /*
                 * no pending messages; schedule possible senders 
                 */
                tcb_set_state(rcv, THREAD_STATE_RECV);
                spinlock_unlock(&rcv->lock);
                sched_switch(cpuid());
                spinlock_lock(&rcv->lock,
                              (unsigned long)sched_get_current_thread(cpuid()));
        }

        if (!rcv->ipcin)
        {
                err = -EAGAIN;
                goto err_rcv_ipcin;
        }

        /*
         * dequeue first IPC message 
         */

        msgin = list_data(rcv->ipcin);
        rcv->ipcin = list_next(rcv->ipcin);

        if (!msgin)
        {
                err = -EAGAIN;
                goto err_msg;
        }

        console_printf("%s:%x msg->flags=%x msgin->flags=%x<\n", __FILE__,
                       __LINE__, msg->flags, msgin->flags);

        if (msg->flags&msgin->flags&IPC_MSG_FLAGS_MMAP)
        {
                /* both threads in mmap mode */

                if (msg->msg1 < msgin->msg1)
                {
                        err = -EAGAIN;
                        goto err_mmap_count;
                }

/*                console_printf("%s:%x: %x\n", __FILE__, __LINE__, msg);
                console_printf("%s:%x: %x\n", __FILE__, __LINE__, msg->snd);
                console_printf("%s:%x: %x\n", __FILE__, __LINE__, msg->snd->task);
                console_printf("%s:%x: %x\n", __FILE__, __LINE__, msg->snd->task->as);*/

                vmem_map_pages_at(rcv->task->as, msg->msg0,
                           msgin->snd->task->as, msgin->msg0,
                                  msgin->msg1, PDE_FLAG_PRESENT |
                                                                PDE_FLAG_WRITEABLE);

                msg->snd = msgin->snd;
                msg->flags = msgin->flags&~IPC_MSG_FLAGS_RESERVED;
                msg->msg1 = msgin->msg1;
        }
        else if (!((msg->flags|msgin->flags)&IPC_MSG_FLAGS_MMAP))
        {
                /* both thread in register mode */
                memcpy(msg, msgin, sizeof(*msg));
        }
        else
        {
                /* distinct modes */

                err = -EINVAL;
                goto err_mode;
        }

        spinlock_unlock(&rcv->lock);

        return 0;

err_mmap_count:
err_mode:
err_msg:
err_rcv_ipcin:
        spinlock_unlock(&rcv->lock);
        return err;
}

int
ipc_reply_and_recv(struct ipc_msg *msg, struct tcb *rcv)
{
        return -ENOSYS;
}

