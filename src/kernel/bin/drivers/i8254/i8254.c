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
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "interupt.h"
#include "ioports.h"
#include "timer.h"

enum i8254_counter {
    i8254_COUNTER_TIMER = 0,
    i8254_COUNTER_DRAM  = 1,
    i8254_COUNTER_SPKR  = 2
};

enum i8254_mode {
    i8254_MODE_TERMINAL = 0x00,
    i8254_MODE_ONESHOT  = 0x01,
    i8254_MODE_RATEGEN  = 0x02,
    i8254_MODE_WAVEGEN  = 0x03,
    i8254_MODE_SWSTROBE = 0x04,
    i8254_MODE_HWSTROBE = 0x05
};

enum {
    i8254_IRQNO = 0
};

enum {
    i8254_DATA0 = 0x40,
    i8254_DATA1 = 0x41,
    i8254_DATA2 = 0x42,
    i8254_CMD   = 0x43
};

enum {
    i8254_TICKS_PER_S = 1193182
};

#define i8254_DATA(_counter) \
    ( i8254_DATA0 + ((_counter) & 0x03) )

struct i8254_drv*
i8254_of_irq_handler(struct irq_handler* irqh)
{
    return containerof(irqh, struct i8254_drv, irq_handler);
}

void
wreg_ticks(enum i8254_counter counter, enum i8254_mode mode, uint16_t ticks)
{
    /* setup PIT control word */
    uint8_t cmd = ((counter & 0x3) << 6) |
                    0x30 |                  /* write LSB, then MSB */
                  ((mode & 0x7) << 1);

    /* setup PIT counter */
    uint8_t lsb = (ticks & 0x00ff);
    uint8_t msb = (ticks & 0xff00) >> 8;

    bool ints_on = cli_if_on();

    io_outb(i8254_CMD, cmd);
    io_outb(i8254_DATA(counter), lsb);
    io_outb(i8254_DATA(counter), msb);

    sti_if_on(ints_on);
}

static enum irq_status
irq_handler_func(unsigned char irqno, struct irq_handler* irqh)
{
    struct i8254_drv* i8254 = i8254_of_irq_handler(irqh);

    unsigned long freq = i8254->counter_freq[i8254_COUNTER_TIMER];

    i8254->timestamp += S_TO_NS(1) / freq;
    handle_timeout(i8254->timestamp);

    return IRQ_HANDLED;
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
 * \brief program i8254 timer channel
 * \param freq the trigger frequency
 */
void
i8254_install_timer(struct i8254_drv* i8254, unsigned long freq)
{
    assert(i8254);

    i8254->counter_freq[i8254_COUNTER_TIMER] = freq;

    /* Program number of PIT ticks between IRQs */
    wreg_ticks(i8254_COUNTER_TIMER, i8254_MODE_RATEGEN,
               i8254_TICKS_PER_S / freq);
}
