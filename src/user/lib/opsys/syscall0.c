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
syscall0(unsigned long r0,
         unsigned long r1,
         unsigned long r2,
         unsigned long r3)
{
        unsigned long flags, errno;

        __asm__("int $0x80\n\t"
                        : "=b"(flags),
                          "=c"(errno)
                        : "a"(r0),
                          "b"(r1),
                          "c"(r2),
                          "d"(r3)
                        : );

        return flags&0x1 ? (int)errno : 0; /* return possible error code */
}

