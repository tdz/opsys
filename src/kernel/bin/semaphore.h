/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2010  Thomas Zimmermann
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

#include "spinlock.h"

struct list;

struct semaphore
{
        spinlock_type lock;
        unsigned long slots;
        struct list *waiters;
};

int
semaphore_init(struct semaphore *sem, unsigned long slots);

void
semaphore_uninit(struct semaphore *sem);

int
semaphore_try_enter(struct semaphore *sem);

void
semaphore_enter(struct semaphore *sem);

void
semaphore_leave(struct semaphore *sem);
