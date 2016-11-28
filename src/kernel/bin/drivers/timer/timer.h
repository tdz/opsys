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

/* relative timeout in nanoseconds */
typedef unsigned long timeout_t;

/* relative timestamp in nanoseconds */
typedef unsigned long timestamp_t;

struct timer_drv {
    int (*set_timeout)(struct timer_drv*, timeout_t);
    void (*clear_timeout)(struct timer_drv*);
};

int
timer_drv_init(struct timer_drv* drv,
              int (*set_timeout)(struct timer_drv*, timeout_t),
              void (*clear_timeout)(struct timer_drv*));

void
timer_drv_uninit(struct timer_drv* timer_drv);

static inline int
timer_drv_set_timeout(struct timer_drv* timer_drv, timeout_t reltime_ns)
{
    return timer_drv->set_timeout(timer_drv, reltime_ns);
}

static inline void
timer_drv_clear_timeout(struct timer_drv* timer_drv)
{
    timer_drv->clear_timeout(timer_drv);
}
