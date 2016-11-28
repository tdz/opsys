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

#include "i8254.h"
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "ioports.h"
#include "timer.h"

enum {
    i8254_IRQNO = 0
};

enum {
    i8254_DATA0 = 0x40,
    i8254_DATA1 = 0x41,
    i8254_DATA2 = 0x42,
    i8254_CMD   = 0x43
};

#define i8254_DATA(_counter) \
    ( i8254_DATA0 + ((_counter) & 0x03) )

static enum irq_status
irq_handler_func(unsigned char irqno, struct irq_handler* irqh)
{
    static unsigned long tickcounter = 0;

    ++tickcounter;

    handle_timeout();

    return IRQ_NOT_HANDLED;
}

static int
set_timeout(struct timer_drv* drv, timeout_t timeout_ns)
{
    return 0;
}

static void
clear_timeout(struct timer_drv* drv)
{ }

int
i8254_init(struct i8254_drv* i8254)
{
    assert(i8254);

    int res = timer_drv_init(&i8254->drv, set_timeout, clear_timeout);
    if (res < 0) {
        return res;
    }

    res = init_timer(&i8254->drv);
    if (res < 0) {
        goto err_init_timer;
    }

    for (size_t i = 0; i < ARRAY_NELEMS(i8254->counter_freq); ++i) {
        i8254->counter_freq[i] = 0;
    }

    irq_handler_init(&i8254->irq_handler, irq_handler_func);

    res = install_irq_handler(i8254_IRQNO, &i8254->irq_handler);
    if (res < 0) {
        goto err_install_irq_handler;
    }

    return 0;

err_install_irq_handler:
    uninit_timer();
err_init_timer:
    timer_drv_uninit(&i8254->drv);
    return res;
}

void
i8254_uninit(struct i8254_drv* i8254)
{
    assert(i8254);

    remove_irq_handler(i8254_IRQNO, &i8254->irq_handler);
    uninit_timer();
    timer_drv_uninit(&i8254->drv);
}

/**
 * \brief program PIT
 * \param counter the PIT counter to use
 * \param freq the trigger frequency
 * \param mode the mode
 */
void
i8254_set_up(struct i8254_drv* i8254, enum pit_counter counter,
             unsigned long freq, enum pit_mode mode)
{
    assert(i8254);

    /* setup PIT control word */

    uint8_t byte = ((counter & 0x3) << 6) |
                   (0x3 << 4) |            /* LSB, then MSB */
                   ((mode & 0x7) << 1);

    io_outb(i8254_CMD, byte);

    /* setup PIT counter */

    uint16_t word = 1193180 / freq;

    uint8_t byte0 = (word & 0x00ff);
    uint8_t byte1 = (word & 0xff00) >> 8;

    io_outb(i8254_DATA(counter), byte0);
    io_outb(i8254_DATA(counter), byte1);

    i8254->counter_freq[counter] = freq;
}
