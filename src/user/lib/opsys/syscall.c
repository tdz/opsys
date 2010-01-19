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

#include <types.h>
#include <syscall.h>

int
syscall_crt_write(const char *buf, size_t buflen, unsigned char attr)
{
        int res;

        __asm__("pushl %4\n\t" /* push attr */
                "pushl %3\n\t" /* push buflen */
                "pushl %2\n\t" /* push buf */
                "pushl %1\n\t" /* push syscall number */
                "int $0x80\n\t"
                "addl $16, %%esp\n\t" /* restore stack pointer */
                        : "=a"(res) /* save result from %eax */
                        : "r"(SYSCALL_CRT_WRITE),
                          "r"(buf),
                          "r"(buflen),
                          "r"((int)attr)
                        : "ecx", "edx");

        return res;
}

