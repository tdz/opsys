/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2010  Thomas Zimmermann
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

#include "pic.h"
#include <stddef.h>
#include <stdint.h>
#include "idt.h"
#include "ioports.h"
#include "irq.h"

static int
enable_irq(unsigned char irqno)
{
    return idt_install_irq_handler(irqno, pic_handle_irq);
}

static void
disable_irq(unsigned char irqno)
{
    idt_install_irq_handler(irqno, NULL);
}

void
pic_install()
{
    /* Init IRQ framework */
    init_irq_handling(enable_irq, disable_irq);

    /* out ICW 1 */
    io_outb(0x20, 0x11);
    io_outb(0xa0, 0x11);

    /* out ICW 2 */
    io_outb(0x21, 0x20);
    io_outb(0xa1, 0x28);

    /* out ICW 3 */
    io_outb(0x21, 0x04);
    io_outb(0xa1, 0x02);

    /* out ICW 4 */
    io_outb(0x21, 0x01);
    io_outb(0xa1, 0x01);
}

void
pic_handle_irq(unsigned char irqno)
{
    /* Call IRQ framework */
    handle_irq(irqno);
}

/* signal end of interupt to PIC */
void
pic_eoi(unsigned char intno)
{
    static const uint8_t ioport[2] = {
        0x20,
        0xa0
    };

    if (intno > 15) {
        return; /* silently ignore software interrupts */
    }

    io_outb(ioport[intno > 7], 0x20);
}
