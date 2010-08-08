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

#include <syscall.h>

int
syscall(unsigned long rcv,
        unsigned long flags,
        unsigned long msg0,
        unsigned long msg1,
        unsigned long *reply_rcv,
        unsigned long *reply_flags,
        unsigned long *reply_msg0, unsigned long *reply_msg1)
{
        __asm__ volatile ("int $0x80\n\t"
                        : "=a"(*reply_rcv),
                          "=b"(*reply_flags),
                          "=c"(*reply_msg0),
                          "=d"(*reply_msg1)
                        : "0"(rcv),
                          "1"(flags),
                          "2"(msg0),
                          "3"(msg1));

        return *reply_flags & 0x1 ? (int)*reply_msg0 : 0;       /* return possible error code */
}

