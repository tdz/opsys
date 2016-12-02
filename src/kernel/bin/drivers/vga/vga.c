/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009  Thomas Zimmermann
 *  Copyright (C) 2016  Thomas Zimmermann
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

#include "vga.h"
#include <stddef.h>
#include "ioports.h"

enum {
    MAX_ROW = 25,
    MAX_COL = 80
};

static unsigned long
vga_getindex(unsigned short row, unsigned short col)
{
    return col + row * MAX_COL;
}

ssize_t
vga_write(volatile unsigned char *vidmem,
          const void *buf, size_t count, unsigned char attr)
{
    size_t i;
    const unsigned char *buf8;

    for (buf8 = buf, i = 0; i < count; ++i) {
        *vidmem = *buf8;
        ++buf8;
        ++vidmem;
        *vidmem = attr;
        ++vidmem;
    }

    return i;
}

int
vga_getpos(unsigned short *row, unsigned short *col)
{
    unsigned long curh = io_inb_index(0x3d4, 0x0e, 0x03d5);
    unsigned long curl = io_inb_index(0x3d4, 0x0f, 0x03d5);

    unsigned long cur = (curh << 8) | curl;

    *row = cur / 80;
    *col = cur - (*row) * 80;

    return 0;
}

int
vga_getmaxpos(unsigned short *row, unsigned short *col)
{
    *row = MAX_ROW;
    *col = MAX_COL;

    return 0;
}

int
vga_setpos(unsigned short row, unsigned short col)
{
    unsigned long cur = vga_getindex(row, col);

    io_outb_index(0x03d4, 0x0e, 0x03d5, (cur >> 8) & 0xff);
    io_outb_index(0x03d4, 0x0f, 0x03d5, cur & 0xff);

    return 0;
}

volatile unsigned char *
vga_getaddress(unsigned short row, unsigned short col)
{
    if ((row >= MAX_ROW) || (col >= MAX_COL)) {
        return NULL;
    }

    return ((volatile unsigned char *)0xb8000) + 2 * vga_getindex(row, col);
}