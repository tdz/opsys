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

#include <sys/types.h>

#include <interupt.h>

#include <list.h>
#include "ipcmsg.h"

#include <tcb.h>

#include "sched.h"
#include "tid.h"
#include "syssrv.h"

#include "console.h"

static int
system_srv_handle_msg(struct ipc_msg *msg)
{
        console_printf("%s:%x.\n", __FILE__, __LINE__);

        switch (msg->msg0) {
                case 0: /* thread quit */
                        /* mark sender thread for removal */
                        tcb_set_state(msg->snd, THREAD_STATE_ZOMBIE);
                        break;
                default:
                        break;
        }

        return 0;
}

#include "console.h"
void
system_srv_start(struct tcb *self)
{
        while (1) {
                struct ipc_msg *msg;

                tcb_set_state(self, THREAD_STATE_RECV);

                console_printf("%s:%x self=%x.\n", __FILE__, __LINE__, self);

                sched_switch(0);
                console_printf("%s:%x.\n", __FILE__, __LINE__);

                if (!self->ipcin) {
                        continue;
                }
                console_printf("%s:%x.\n", __FILE__, __LINE__);

                /* deque first IPC message */                

                console_printf("%s:%x.\n", __FILE__, __LINE__);
                cli();
                console_printf("%s:%x.\n", __FILE__, __LINE__);
                msg = list_data(self->ipcin);
                console_printf("%s:%x.\n", __FILE__, __LINE__);
                list_deque(self->ipcin);
                console_printf("%s:%x.\n", __FILE__, __LINE__);
                sti();
                console_printf("%s:%x.\n", __FILE__, __LINE__);

                if (!msg) {
                        continue;
                }
                console_printf("%s:%x.\n", __FILE__, __LINE__);

                system_srv_handle_msg(msg);
                console_printf("%s:%x.\n", __FILE__, __LINE__);
        }
}

