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
spinlock_init(struct spinlock *spinlock)
{
        spinlock->owner = NULL;

        return 0;
}

void
spinlock_uninit(struct spinlock *spinlock)
{
        return;
}

int
spinlock_lock(struct spinlock *spinlock, const struct tcb *self)
{
        int err;

        do {
                err = spinlock_try_lock(spinlock, self);
        } while (err == -EBUSY);

        return err;
}

int
spinlock_try_lock(struct spinlock *spinlock, const struct tcb *self)
{
        const struct tcb *owner;
        
        owner = (const struct tcb *)atomic_xchg(&spinlock->owner,
                                                (unsigned long)self);

        return owner ? -EBUSY : 0;
}

void
spinlock_unlock(struct spinlock *spinlock)
{
        spinlock->owner = NULL;
}

