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

struct task;

#include "ipcmsg.h"
#include "list.h"
#include "spinlock.h"
#include "tcbregs.h"

enum thread_state
{
        THREAD_STATE_ZOMBIE = 0, /**< \brief waiting for removal */
        THREAD_STATE_READY, /**< \brief ready to be executed */
        THREAD_STATE_SEND, /**< \brief blocked by send action */
        THREAD_STATE_RECV, /**< \brief blocked by receive action */
        THREAD_STATE_WAITING /**< \brief blocked by waiting for some lock */
};

struct tcb
{
    enum thread_state state;
    struct task *task; /**< Address of task structure of the thread */
    void *stack; /**< Stack base address */
    unsigned char id; /**< Task-local id */

    struct tcb_regs regs; /**< CPU registers */

    struct list * volatile ipcin; /**< List of incoming IPC messages */
    struct ipc_msg msg;
    struct list wait;
    struct list sched;

    spinlock_type lock;
};

struct tcb*
tcb_of_sched_list(struct list* l);

struct tcb*
tcb_of_wait_list(struct list* l);

int
tcb_init_with_id(struct tcb *tcb,
                 struct task *task, unsigned char id, void *stack);

int
tcb_init(struct tcb *tcb, struct task *task, void *stack);

void
tcb_uninit(struct tcb *tcb);

void
tcb_set_state(struct tcb *tcb, enum thread_state state);

enum thread_state
tcb_get_state(const struct tcb *tcb);

int
tcb_set_initial_ready_state(struct tcb *tcb,
                            const void *ip,
                            unsigned char irqno,
                            unsigned long *stack, int nargs, ...);

int
tcb_is_runnable(const struct tcb *tcb);

int
tcb_switch(struct tcb *tcb, const struct tcb *dst);
