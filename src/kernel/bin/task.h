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

#include <sys/types.h>

struct vmem;

struct task
{
        struct vmem *as;
        unsigned int  id;
        unsigned char nthreads;
        unsigned char threadid[8];
};

int
task_init(struct task *task, struct vmem *as);

void
task_uninit(struct task *task);

size_t
task_max_nthreads(const struct task *task);

int
task_ref(struct task *task);

int
task_unref(struct task *task);
