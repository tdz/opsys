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

#define PIC1_CMD    0x20
#define PIC1_DATA   0x21
#define PIC2_CMD    0xa0
#define PIC2_DATA   0xa1

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

    /* Send init code 0x11 to PICs */
    io_outb(PIC1_CMD, 0x11);
    io_outb(PIC2_CMD, 0x11);

    /* Remap interupt numbers to not collide with CPU exceptions */
    io_outb(PIC1_DATA, IDT_IRQ_OFFSET);
    io_outb(PIC2_DATA, IDT_IRQ_OFFSET + 8);

    /* Master-slave configuration */
    io_outb(PIC1_DATA, 0x04); /* Set PIC1 to master */
    io_outb(PIC2_DATA, 0x02); /* Set PIC2 to slave */

    /* Set x86 mode */
    io_outb(PIC1_DATA, 0x01);
    io_outb(PIC1_DATA, 0x01);
}

void
pic_handle_irq(unsigned char irqno)
{
    /* Call IRQ framework */
    handle_irq(irqno);
}

/* signal end of interupt to PIC */
void
pic_eoi(unsigned char irqno)
{
    if (irqno > 15) {
        /* silently ignore software interrupts */
        return;
    } else if (irqno > 7) {
        /* Signal EOI for PIC2 */
        io_outb(PIC2_CMD, 0x20);
    }

    /* Always signal EOI to PIC1 */
    io_outb(PIC1_CMD, 0x20);
}
