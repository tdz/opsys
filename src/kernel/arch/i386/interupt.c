/*
 *  opsys - A small, experimental operating system
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

#include "interupt.h"
#include "ioports.h"

/* clear interupts */
void
cli()
{
        __asm__("cli\n\t");
}

/* set interupts */
void
sti()
{
        __asm__("sti\n\t");
}

/* signal end of interupt to PIC */
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

int
int_enabled()
{
        unsigned long eflags;

        __asm__("pushf\n\t"
                "movl (%%esp), %0\n\t"
                "popf\n\t"
                        : "=r"(eflags));

        return !!(eflags & (1<<9));
}

