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

#include "ipc.h"
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include "cpu.h"
#include "sched.h"
#include "task.h"
#include "tcb.h"
#include "vmem.h"

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

        spinlock_lock(&rcv->lock, (unsigned long)sched_get_current_thread(cpuid()));
        list_enqueue_back(&rcv->ipcin, &msg->rcv_q);
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
#include "pde.h"
#include "console.h"
int
ipc_recv(struct ipc_msg *msg, struct tcb *rcv)
{
        int err;
        const struct ipc_msg *msgin;

        spinlock_lock(&rcv->lock, (unsigned long)sched_get_current_thread(cpuid()));

        if (list_is_empty(&rcv->ipcin))
        {
                console_printf("%s:%x\n", __FILE__, __LINE__);
                /*
                 * no pending messages; schedule possible senders
                 */
                tcb_set_state(rcv, THREAD_STATE_RECV);
                spinlock_unlock(&rcv->lock);
                sched_switch(cpuid());
                spinlock_lock(&rcv->lock,
                              (unsigned long)sched_get_current_thread(cpuid()));
                console_printf("%s:%x\n", __FILE__, __LINE__);
        }

        if (list_is_empty(&rcv->ipcin))
        {
                console_printf("%s:%x\n", __FILE__, __LINE__);
                err = -EAGAIN;
                goto err_rcv_ipcin;
        }

        /*
         * dequeue first IPC message
         */

        struct list* ipcin = list_first(&rcv->ipcin);
        list_dequeue(ipcin);

        msgin = ipc_msg_of_list(ipcin);

        if (!msgin)
        {
                console_printf("%s:%x\n", __FILE__, __LINE__);
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
