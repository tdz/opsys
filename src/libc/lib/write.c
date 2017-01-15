/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009-2010  Thomas Zimmermann
 *  Copyright (C) 2016-2017  Thomas Zimmermann
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

#include "crt.h"
#include <errno.h>
#include <tid.h>
#include "syscall.h"

int
crt_write(const char *buf, size_t buflen, unsigned char attr)
{
        return syscall0(threadid_create(0, 1),
                        (unsigned long)((SYSCALL_OP_SEND_AND_WAIT<<28)|IPC_MSG_FLAGS_MMAP|1),
                        ((unsigned long)buf)>>12,
                        (unsigned long)(buflen>>12)+1);
}
