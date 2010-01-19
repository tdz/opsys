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

#include "types.h"
#include "syscall.h"
#include "crt.h"

#include "console.h"

int
syscall_task_quit()
{
        console_printf("%s:%x\n", __FILE__, __LINE__);
        return 0;
}

int
syscall_crt_write(const char *buf, size_t buflen, unsigned char attr)
{
        unsigned short row, col;
        volatile unsigned char *vidmem;

        console_printf("%s:%x\n", __FILE__, __LINE__);

        crt_getpos(&row, &col);
        vidmem = crt_getaddress(row, col);

        return crt_write(vidmem, buf, buflen, attr);
}

int
syscall_crt_getmaxpos(unsigned short *row, unsigned short *col)
{
        console_printf("%s:%x\n", __FILE__, __LINE__);
        return 0;
}

int
syscall_crt_getpos(unsigned short *row, unsigned short *col)
{
        console_printf("%s:%x\n", __FILE__, __LINE__);
        return 0;
}

int
syscall_crt_setpos(unsigned short row, unsigned short col)
{
        console_printf("%s:%x\n", __FILE__, __LINE__);
        return 0;
}

