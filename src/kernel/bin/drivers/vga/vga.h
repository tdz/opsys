/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009-2010  Thomas Zimmermann
 *  Copyright (C) 2016       Thomas Zimmermann
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

#pragma once

#include <sys/types.h>

ssize_t
vga_write(volatile unsigned char *vidmem,
          const void *buf,
          size_t count,
          unsigned char attr);

int
vga_getmaxpos(unsigned short *row, unsigned short *col);

int
vga_getpos(unsigned short *row, unsigned short *col);

int
vga_setpos(unsigned short row, unsigned short col);

volatile unsigned char *
vga_getaddress(unsigned short row, unsigned short col);