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

#include "ipcmsg.h"

/* sender waits for reply */
#define IPC_MSG_FLAGS_TIMEOUT(x_)          (((x_)&0xfffffffe)>>1)

int
ipc_msg_init(struct ipc_msg *msg, struct tcb *snd,
                                  unsigned long flags,
                                  unsigned long msg0,
                                  unsigned long msg1)
{
        msg->snd = snd;
        msg->flags = flags;
        msg->msg0 = msg0;
        msg->msg1 = msg1;

        return 0;
}

int
ipc_msg_has_timeout(struct ipc_msg *msg)
{
        return (IPC_MSG_FLAGS_TIMEOUT(msg->flags) != IPC_TIMEOUT_NOW) &&
               (IPC_MSG_FLAGS_TIMEOUT(msg->flags) != IPC_TIMEOUT_NEVER);
}

