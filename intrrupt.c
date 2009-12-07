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

#include "intrrupt.h"
#include "ioports.h"

/* clear interrupts */
void
cli()
{
        __asm__("cli\n\t");
}

/* set interrupts */
void
sti()
{
        __asm__("sti\n\t");
}

/* signal end of interrupt to PIC */
void
eoi(unsigned char intno)
{
        if (intno > 15) {
                return;
        } else if (intno > 7) {
                io_outb(0xa0, 0x20);
        } else {
                io_outb(0x20, 0x20);
        }
}

