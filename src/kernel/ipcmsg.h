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

enum ipc_timeout {
        IPC_TIMEOUT_NOW   = 0,
        IPC_TIMEOUT_NEVER = 0x7fffffff
};

enum ipc_msg_flags {
        IPC_MSG_FLAG_IS_ERRNO = 1<<0
};

struct ipc_msg {
        struct tcb   *snd;
        unsigned long flags;
        unsigned long msg0;
        unsigned long msg1;
};

int
ipc_msg_init(struct ipc_msg *msg, struct tcb   *snd,
                                  unsigned long flags,
                                  unsigned long msg0,
                                  unsigned long msg1);

int
ipc_msg_flags_has_timeout_value(const struct ipc_msg *msg);

unsigned long
ipc_msg_flags_get_timeout(const struct ipc_msg *msg);

int
ipc_msg_flags_is_errno(const struct ipc_msg *msg);

