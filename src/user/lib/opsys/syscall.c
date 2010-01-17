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

void
crt_write(const char *str, size_t len)
{
        __asm__("pushl %1\n\t" /* push len */
                "pushl %0\n\t" /* push str */
                "pushl $1\n\t" /* push syscall number */
                "int $0x80\n\t"
                "addl $12, %%esp\n\t" /* restore stack pointer */
                        :
                        : "r"(str), "r"(len));
}

