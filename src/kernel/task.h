/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009-2010  Thomas Zimmermann <tdz@users.sourceforge.net>
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

struct task
{
        struct page_directory *pd;
        unsigned int  id;
        unsigned char nthreads;
        unsigned char threadid[8];
};

int
task_init(struct task *task, struct page_directory *pd);

int
task_init_from_parent(struct task *tsk, const struct task *parent);

void
task_uninit(struct task *task);

int
task_ref(struct task *task);

int
task_unref(struct task *task);

