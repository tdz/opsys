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

#include <sys/types.h>
#include <syscall.h>

int
syscall(unsigned long r0,
        unsigned long r1,
        unsigned long r2,
        unsigned long r3)
{
        int res;

        __asm__("int $0x80\n\t"
/*                "dohlt: hlt\n\t"
                "jmp dohlt\n\t" */
                        : "=a"(res) /* save result from %eax */
                        : "a"(r0),
                          "b"(r1),
                          "c"(r2),
                          "d"(r3)
                        : );

        return res;
}

int
syscall_crt_write(const char *buf, size_t buflen, unsigned char attr)
{
        return syscall((unsigned long)SYSCALL_CRT_WRITE,
                       (unsigned long)buf,
                       (unsigned long)buflen,
                       (unsigned long)attr);
}

