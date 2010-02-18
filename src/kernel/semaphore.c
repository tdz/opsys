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
#include <sys/types.h>

#include <interupt.h>

#include "list.h"
#include "ipcmsg.h"

#include "spinlock.h"

#include "tcb.h"

#include "sched.h"

#include "semaphore.h"

int
semaphore_init(struct semaphore *sem, unsigned long slots)
{
        int err;

        if ((err = spinlock_init(&sem->lock)) < 0) {
                goto err_spinlock_init;
        }

        sem->slots = slots;

err_spinlock_init:
        return err;
}

void
semaphore_uninit(struct semaphore *sem)
{
        spinlock_uninit(&sem->lock);
}

int
semaphore_enter(struct semaphore *sem, struct tcb *self)
{
        int avail;

        do {
                int int_enable;

                int_enable = int_enabled();

                if (int_enable) {
                        cli();
                }

                spinlock_lock(&sem->lock, (unsigned long)self);

                avail = !!sem->slots;

                if (avail) {
                        /* empty slots available */
                        --sem->slots;
                } else {
                        struct list *waiters;

                        /* no slots available, enque self in waiter list */

                        if (sem->waiters) {
                                waiters = list_init(&self->wait,
                                                    sem->waiters->prev,
                                                    sem->waiters,
                                                    self);
                        } else {
                                waiters = list_init(&self->wait,
                                                    NULL,
                                                    NULL,
                                                    self);
                        }

                        sem->waiters = waiters;

                        tcb_set_state(self, THREAD_STATE_WAITING);
                }

                spinlock_unlock(&sem->lock);

                if (int_enable) {
                        sti();
                }

                if (!avail) {
                        /* no slots available, schedule self */
                        sched_switch(0);

                        /* when returning, self is not waiting anymore */

                        if (int_enable) {
                                cli();
                        }

                        spinlock_lock(&sem->lock, (unsigned long)self);

                        if (sem->waiters == &self->wait) {
                                sem->waiters = self->wait.next;
                        }
                        list_deque(&self->wait);

                        spinlock_unlock(&sem->lock);

                        if (int_enable) {
                                sti();
                        }
                }
        } while (!avail);

        return 0;
}

int
semaphore_try_enter(struct semaphore *sem, struct tcb *self)
{
        int avail;
        int int_enable;

        int_enable = int_enabled();

        if (int_enable) {
                cli();
        }

        spinlock_lock(&sem->lock, (unsigned long)self);

        avail = !!sem->slots;

        if (avail) {
                --sem->slots;
        }

        spinlock_unlock(&sem->lock);

        if (int_enable) {
                sti();
        }

        return avail ? 0 : -EBUSY;
}

void
semaphore_leave(struct semaphore *sem, struct tcb *self)
{
        struct list *waiters;

        int int_enable = int_enabled();

        if (int_enable) {
                cli();
        }

        spinlock_lock(&sem->lock, (unsigned long)self);

        /* free slot */
        ++sem->slots;

        /* wake up one waiter */

        for (waiters = sem->waiters; waiters; waiters = list_next(waiters)) {
                struct tcb *waiter = list_data(sem->waiters);

                /* any thread in this list is either blocked or has been
                   woken up by some other, concurrent invocation of this
                   function, therefore only consider waiting threads */

                if (tcb_get_state(waiter) == THREAD_STATE_WAITING) {
                        tcb_set_state(waiter, THREAD_STATE_READY);
                        break;
                }
        }

        spinlock_unlock(&sem->lock);

        if (int_enable) {
                sti();
        }
}
