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

struct list;
struct tcb;

typedef unsigned char prio_class_type;

enum
{
        SCHED_NPRIOS = 1<<(sizeof(prio_class_type)*8)
};

int
sched_init(unsigned int cpu, struct tcb *idle);

int
sched_add_thread(struct tcb *tcb, prio_class_type prio);

struct tcb *
sched_get_current_thread(void);

struct tcb *
sched_get_thread(struct list *listhead);

struct tcb *
sched_search_thread(unsigned int taskid, unsigned char tcbid);

int
sched_switch_to(struct tcb *next);

int
sched_switch(void);

void
sched_irq_handler(unsigned char irqno);

