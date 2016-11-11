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

#include <sys/types.h>

struct task;
struct tcb;

int
tcb_helper_allocate_tcb(struct task *task, void *stack, struct tcb **tcb);

int
tcb_helper_allocate_tcb_and_stack(struct task *tsk, size_t stackpages,
                                  struct tcb **tcb);

int
tcb_helper_run_kernel_thread(struct tcb *tcb, void (*func)(struct tcb*));

int
tcb_helper_run_user_thread(struct tcb *cur_tcb, struct tcb *usr_tcb, void *ip);
