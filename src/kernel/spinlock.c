/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2010  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include <errno.h>
#include <stddef.h>

#include <atomic.h>

#include "spinlock.h"

int
spinlock_init(spinlock_type *spinlock)
{
        *spinlock = 0;

        return 0;
}

void
spinlock_uninit(spinlock_type *spinlock)
{
        return;
}

int
spinlock_lock(spinlock_type *spinlock, unsigned long unique_id)
{
        int err;

        do {
                err = spinlock_try_lock(spinlock, unique_id);
        } while (err == -EBUSY);

        return err;
}

int
spinlock_try_lock(spinlock_type *spinlock, unsigned long unique_id)
{
        unsigned long was_locked;

        if (*spinlock) {
                /* don't try if already locked, prevents bus lock */
                was_locked = 1;
        } else {
                was_locked = atomic_xchg(spinlock, unique_id);
        }

        return was_locked ? -EBUSY : 0;
}

void
spinlock_unlock(spinlock_type *spinlock)
{
        *spinlock = 0;
}

