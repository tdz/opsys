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

#include "types.h"
#include "entry.h"
#include <syscall.h>

#include "console.h"

int
entry_task_quit(unsigned long eip,
                unsigned long cs,
                unsigned long eflags,
                unsigned long sysno)
{
        return syscall_task_quit();
}

int
entry_crt_write(unsigned long eip,
                unsigned long cs,
                unsigned long eflags,
                unsigned long sysno,
                unsigned long buf, unsigned long buflen, unsigned long attr)
{
        return syscall_crt_write((const char*)buf,
                                 (size_t)buflen,
                                 (unsigned char)attr);
}

int
entry_crt_getmaxpos(unsigned long eip,
                      unsigned long cs,
                      unsigned long eflags,
                      unsigned long sysno,
                      unsigned long row, unsigned long col)
{
        return syscall_crt_getmaxpos((unsigned short*)row,
                                     (unsigned short*)col);
}

int
entry_crt_getpos(unsigned long eip,
                   unsigned long cs,
                   unsigned long eflags,
                   unsigned long sysno,
                   unsigned long row, unsigned long col)
{
        return syscall_crt_getpos((unsigned short*)row, (unsigned short*)col);
}

int
entry_crt_setpos(unsigned long eip,
                 unsigned long cs,
                 unsigned long eflags,
                 unsigned long sysno,
                 unsigned long row, unsigned long col)
{
        return syscall_crt_setpos((unsigned short)row, (unsigned short)col);
}

