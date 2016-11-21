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

#include "list.h"

/* relative timeout in nanoseconds */
typedef unsigned long timeout_t;

/* relative timestamp in nanoseconds */
typedef unsigned long timestamp_t;

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
init_timer(void);

void
uninit_timer(void);

int
timer_add_alarm(struct alarm* alarm, timeout_t reltime_ns);

void
timer_remove_alarm(struct alarm* alarm);
