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

#include "pit.h"

void
pit_install(unsigned int counter, unsigned long freq, enum pit_mode mode)
{
        unsigned char byte;
        unsigned short word;

        /* setup PIT control word */

        byte = ((counter&0x3) << 6) |
             (0x3 << 4) | /* LSB then MSB */
             ((mode&0x7) << 1);

        io_outb(0x43, byte);

        /* setup PIT counter */

        word = 1193180 / freq;

        io_outb(0x40+(counter&0x03), word&0xff);
        io_outb(0x40+(counter&0x03), (word&0xff00)>>8);
}

