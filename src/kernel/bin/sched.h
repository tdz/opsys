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

struct tcb;

/** \brief a priority class */
typedef unsigned char prio_class_type;

enum {
    /** \brief frequency for thread scheduling in Hz */
    SCHED_FREQ = 20,
    /** \brief maximum number of CPUs supported by scheduler */
    SCHED_NCPUS  = 1,
    /** \brief number of priority classes supported by scheduler */
    SCHED_NPRIOS = 1 << (sizeof(prio_class_type) * 8)
};

int
sched_init(struct tcb* idle);

int
sched_add_thread(struct tcb* tcb, prio_class_type prio);

struct tcb*
sched_get_current_thread(unsigned int cpu);

struct tcb*
sched_search_thread(unsigned int taskid, unsigned char tcbid);

int
sched_switch_to(unsigned int cpu, struct tcb* next);

int
sched_switch(unsigned int cpu);
