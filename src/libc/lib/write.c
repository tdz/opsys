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
#include <sys/types.h>
#include <generic/tid.h>
#include <syscall.h>
#include <crt.h>

enum syscall_op
{
        SYSCALL_OP_SEND, /**< send message to another thread */
        SYSCALL_OP_SEND_AND_WAIT, /**< send message to another thread and wait for its answer */
        SYSCALL_OP_RECV, /**< receive from any thread */
        SYSCALL_OP_REPLY_AND_RECV /**< replay to thread and receive from any thread */
};

enum ipc_msg_flags
{
        IPC_MSG_FLAGS_RESERVED = 0xe0000000,
        IPC_MSG_FLAGS_MMAP     = 1<<17,
        IPC_MSG_FLAG_IS_ERRNO  = 1<<16
};

int
crt_write(const char *buf, size_t buflen, unsigned char attr)
{
        return syscall0(threadid_create(0, 1),
                        (unsigned long)((SYSCALL_OP_SEND_AND_WAIT<<28)|IPC_MSG_FLAGS_MMAP|1),
                        ((unsigned long)buf)>>12,
                        (unsigned long)(buflen>>12)+1);
}

