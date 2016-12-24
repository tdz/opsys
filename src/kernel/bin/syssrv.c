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

#include "syssrv.h"
#include <errno.h>
#include "console.h"
#include "ipc.h"
#include "task.h"
#include "tcb.h"
#include "vmem.h"

static int
system_srv_handle_msg(struct ipc_msg *msg, struct tcb *self)
{
        console_printf("%s:%x.\n", __FILE__, __LINE__);

        switch (msg->flags&0xffff)
        {
                case 0:        /* thread quit */
                        /*
                         * mark sender thread for removal
                         */
                        console_printf("%s:%x.\n", __FILE__, __LINE__);
                        tcb_set_state(msg->snd, THREAD_STATE_ZOMBIE);
                        break;
                case 1:        /* write to console */
                        /*
                         * mark sender thread for removal
                         */
                        console_printf("%s:%x\n", __FILE__, __LINE__);
                        console_printf("received msg: %s\n", (msg->msg0)<<12);

                                struct tcb *rcv = msg->snd;
                                ipc_msg_init(msg, self,
                                             IPC_MSG_FLAG_IS_ERRNO,
                                             ENOSYS, 0);
                                ipc_reply(msg, rcv);

                        break;
                default:
                        /*
                         * unknown command
                         */
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
        while (1)
        {
                int err;
                struct ipc_msg msg;

                msg.flags = IPC_MSG_FLAGS_MMAP;
                msg.msg1 = 1;
                msg.msg0 = vmem_empty_pages_in_area(self->task->as,
                                                    VMEM_AREA_KERNEL,
                                                    msg.msg1);

                console_printf("%s:%x syssrv=%x.\n", __FILE__, __LINE__,
                               self);

                if ((err = ipc_recv(&msg, self)) < 0)
                {
                        goto err_ipc_recv;
                }

                if ((err = system_srv_handle_msg(&msg, self)) < 0)
                {
                        goto err_system_srv_handle_msg;
                }

/*                console_printf("%s:%x.\n", __FILE__, __LINE__);*/

                continue;

        err_system_srv_handle_msg:
        err_ipc_recv:
                continue;
        }
}
