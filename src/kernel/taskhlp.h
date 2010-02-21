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

struct address_space;
struct task;

int
task_helper_init_kernel_task(struct address_space *kernel_as,
                             struct task **tsk);

int
task_helper_allocate_task_from_parent(const struct task *parent,
                                            struct task **tsk);

int
task_helper_init_task_from_parent(const struct task *parent,
                                        struct task *tsk);

