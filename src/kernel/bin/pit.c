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

/**
 * \file pit.c
 * \author Thomas Zimmermann <tdz@users.sourceforge.net>
 * \date 2010
 *
 * The Programmable Interval Timer (abbr. PIT) is the oldest timer of
 * the IBM PC. The chip is an Intel 8254, or compatible. It has three
 * individual counters for
 *
 *  - generating timer interrupts,
 *  - configuring DRAM refresh, and
 *  - programming the PC speaker.
 *
 * Only the first and third are relevant anymore.
 *
 * \see http://www.stanford.edu/class/cs140/projects/pintos/specs/8254.pdf
 */

#include <ioports.h>
#include "pit.h"

/**
 * \brief program PIT
 * \param counter the PIT counter to use
 * \param freq the trigger frequency
 * \param mode the mode
 */
void
pit_install(enum pit_counter counter, unsigned long freq, enum pit_mode mode)
{
        unsigned char byte;
        unsigned short word;

        /* setup PIT control word 
         */

        byte = ((counter & 0x3) << 6) | (0x3 << 4) |    /* LSB then MSB */
                ((mode & 0x7) << 1);

        io_outb(0x43, byte);

        /* setup PIT counter 
         */

        word = 1193180 / freq;

        io_outb(0x40 + (counter & 0x03), word & 0xff);
        io_outb(0x40 + (counter & 0x03), (word & 0xff00) >> 8);
}

#include "console.h"

void
pit_irq_handler(unsigned char irqno)
{
        static unsigned long tickcounter = 0;

/*        console_printf("%s:%x timer handler.\n", __FILE__, __LINE__);*/

        ++tickcounter;
}
