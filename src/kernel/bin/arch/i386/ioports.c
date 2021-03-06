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

#include "ioports.h"

void
io_outb(unsigned short port, unsigned char byte)
{
    __asm__("mov %1, %%al\n\t"
            "mov %0, %%dx\n\t"
            "out %%al, %%dx\n"
            :
            : "r"(port), "r"(byte)
            : "%al", "%dx");
}

void
io_outb_index(unsigned short iport, unsigned char index,
              unsigned short dport, unsigned char byte)
{
    io_outb(iport, index);
    io_outb(dport, byte);
}

unsigned char
io_inb(unsigned short port)
{
    unsigned char byte;

    __asm__("mov %1, %%dx\n\t"
            "in %%dx, %%al\n\t"
            "mov %%al, %0\n"
            : "=r"(byte)
            : "r"(port)
            : "%al", "%dx");

    return byte;
}

unsigned char
io_inb_index(unsigned short iport, unsigned char index, unsigned short dport)
{
    io_outb(iport, index);

    return io_inb(dport);
}
