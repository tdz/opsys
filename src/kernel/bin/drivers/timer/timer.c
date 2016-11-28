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

#include "timer.h"
#include <assert.h>

int
timer_drv_init(struct timer_drv* timer_drv,
              int (*set_timeout)(struct timer_drv*, timeout_t),
              void (*clear_timeout)(struct timer_drv*))
{
    assert(timer_drv);

    timer_drv->set_timeout = set_timeout;
    timer_drv->clear_timeout = clear_timeout;

    return 0;
}

void
timer_drv_uninit(struct timer_drv* timer_drv)
{
    assert(timer_drv);
}
