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
#include <stddef.h>

/* sender waits for reply */
#define IPC_MSG_FLAGS_TIMEOUT(x_)          (((x_)&0xfffffffe)>>1)

struct ipc_msg*
ipc_msg_of_list(struct list* l)
{
    return containerof(l, struct ipc_msg, rcv_q);
}

int
ipc_msg_init(struct ipc_msg *msg, struct tcb *snd,
             unsigned long flags, unsigned long msg0, unsigned long msg1)
{
    list_init(&msg->rcv_q, NULL, NULL, &msg->rcv_q);
    msg->snd= snd;
    msg->flags = flags&~IPC_MSG_FLAGS_RESERVED;
    msg->msg0 = msg0;
    msg->msg1 = msg1;

    return 0;
}

int
ipc_msg_flags_has_timeout_value(const struct ipc_msg *msg)
{
        unsigned long timeout = ipc_msg_flags_get_timeout(msg);

        return (timeout != IPC_TIMEOUT_NOW) && (timeout != IPC_TIMEOUT_NEVER);
}

unsigned long
ipc_msg_flags_get_timeout(const struct ipc_msg *msg)
{
        return IPC_MSG_FLAGS_TIMEOUT(msg->flags);
}

int
ipc_msg_flags_is_errno(const struct ipc_msg *msg)
{
        return !!(msg->flags & IPC_MSG_FLAG_IS_ERRNO);
}
