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
#include <sys/types.h>

#include <interupt.h>

#include "list.h"
#include "ipcmsg.h"
#include "ipc.h"

#include <tcb.h>

#include "sched.h"
#include "tid.h"
#include "syssrv.h"

#include "console.h"

static int
system_srv_handle_msg(struct ipc_msg *msg, struct tcb *self)
{
        console_printf("%s:%x.\n", __FILE__, __LINE__);

        switch (msg->msg0) {
                case 0: /* thread quit */
                        /* mark sender thread for removal */
                        tcb_set_state(msg->snd, THREAD_STATE_ZOMBIE);
                        break;
                default:
                        /* unknown command */
                        {
                                struct tcb *rcv = msg->snd;
                                ipc_msg_init(msg, self,
                                                  IPC_MSG_FLAG_IS_ERRNO,
                                                  ENOSYS, 0);
                                ipc_reply(msg, rcv);
                        }
                        break;
        }

        return 0;
}

void
system_srv_start(struct tcb *self)
{
        while (1) {
                int err;
                struct ipc_msg *msg;

                console_printf("%s:%x self=%x.\n", __FILE__, __LINE__, self);

                if ((err = ipc_recv(&msg, self)) < 0) {
                        goto err_ipc_recv;
                }

                if ((err = system_srv_handle_msg(msg, self)) < 0) {
                        goto err_system_srv_handle_msg;
                }

                console_printf("%s:%x.\n", __FILE__, __LINE__);

                continue;

        err_system_srv_handle_msg:
        err_ipc_recv:
                continue;
        }
}

