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

#include "irq.h"
#include "drivers/timer/timer.h"

enum pit_counter {
    PIT_COUNTER_TIMER = 0,
    PIT_COUNTER_DRAM  = 1,
    PIT_COUNTER_SPKR  = 2
};

enum pit_mode {
    PIT_MODE_TERMINAL = 0x00,
    PIT_MODE_ONESHOT  = 0x01,
    PIT_MODE_RATEGEN  = 0x02,
    PIT_MODE_WAVEGEN  = 0x03,
    PIT_MODE_SWSTROBE = 0x04,
    PIT_MODE_HWSTROBE = 0x05
};

struct i8254_drv {
    struct timer_drv drv;

    unsigned long counter_freq[3];

    struct irq_handler irq_handler;
};

int
i8254_init(struct i8254_drv* i8254);

void
i8254_uninit(struct i8254_drv* i8254);

void
i8254_set_up(struct i8254_drv* i8254, enum pit_counter counter,
             unsigned long freq, enum pit_mode mode);
