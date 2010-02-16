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

#include <interupt.h>

#include "list.h"

#include "ipcmsg.h"
#include "ipc.h"

#include <tcb.h>

#include "sched.h"

int
ipc_send_and_recv(struct ipc_msg *msg, struct tcb *rcv)
{
        if (rcv->ipcin) {
                rcv->ipcin = list_init(&msg->snd->ipcout, rcv->ipcin->prev,
                                                          rcv->ipcin,
                                                         &msg->snd->msg);
        } else {
                rcv->ipcin = list_init(&msg->snd->ipcout,
                                        NULL,
                                        NULL,
                                       &msg->snd->msg);
        }

        /* sender state */

        tcb_set_state(msg->snd, THREAD_STATE_RECV);

        if (ipc_msg_has_timeout(msg)) {
                /* TODO: implement timeout */
        }

        /* wake up receiver if necessary, and schedule */

        if (tcb_get_state(rcv) == THREAD_STATE_RECV) {
                tcb_set_state(rcv, THREAD_STATE_READY);
        }

        sched_switch(1);

        return 0;
}

int
ipc_recv(struct ipc_msg **msg, struct tcb *rcv)
{
        int err;

        tcb_set_state(rcv, THREAD_STATE_RECV);

        /* schedule possible senders */
        sched_switch(0);

        if (!rcv->ipcin) {
                err = -EAGAIN;
                goto err_rcv_ipcin;
        }

        /* deque first IPC message */                

        cli();
        *msg = list_data(rcv->ipcin);
        list_deque(rcv->ipcin);
        sti();

        if (!*msg) {
                err = -EAGAIN;
                goto err_msg;
        }

        return 0;

err_msg:
err_rcv_ipcin:
        return err;
}

