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

int
taskmngr_init();

ssize_t
taskmngr_allocate_task(struct task *parent);

ssize_t
taskmngr_add_task(struct task *tsk);

struct task *
taskmngr_get_current_task(void);

struct task *
taskmngr_get_task(unsigned int taskid);

struct tcb *
taskmngr_get_current_tcb(void);

struct tcb *
taskmngr_get_tcb(threadid_type tid);

struct tcb *
taskmngr_switchto(struct tcb *tcb);

