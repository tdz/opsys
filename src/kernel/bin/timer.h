/*
 *  opsys - A small, experimental operating system
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

#pragma once

#include "drivers/timer/timer.h"
#include "list.h"

#define S_TO_MS(_s)     (1000 * (_s))
#define MS_TO_uS(_ms)   (1000 * (_ms))
#define uS_TO_NS(_us)   (1000 * (_us))

#define S_TO_NS(_s)     uS_TO_NS(MS_TO_uS(S_TO_MS(_s)))

struct alarm {
    struct list timer_entry;

    timestamp_t timestamp_ns;
    timeout_t (*func)(struct alarm*);
};

int
alarm_init(struct alarm* alarm, timeout_t (*func)(struct alarm*));

bool
alarm_has_expired(const struct alarm* alarm, timestamp_t timestamp_ns);

int
init_timer(struct timer_drv* drv);

void
uninit_timer(void);

void
handle_timeout(timestamp_t timestamp_ns);

int
timer_add_alarm(struct alarm* alarm, timeout_t reltime_ns);

void
timer_remove_alarm(struct alarm* alarm);
