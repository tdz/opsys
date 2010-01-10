/*
 *  oskernel - A small experimental operating-system kernel
 *  Copyright (C) 2009  Thomas Zimmermann <tdz@users.sourceforge.net>
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

int
syscall_crt_write(const void *buf, unsigned long count, unsigned char attr)
{
        unsigned short row, col;
        volatile unsigned char *vidmem;

        crt_getpos(&row, &col);
        vidmem = crt_getaddress(row, col);

        return crt_write(vidmem, buf, (size_t)count, attr);
}

int
syscall_crt_getmaxpos(unsigned short *row, unsigned short *col)
{
        return 0;
}

int
syscall_crt_getpos(unsigned short *row, unsigned short *col)
{
        return 0;
}

int
syscall_crt_setpos(unsigned short row, unsigned short col)
{
        return 0;
}
