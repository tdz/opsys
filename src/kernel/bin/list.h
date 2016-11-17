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

#include <stdbool.h>

struct list {
    struct list* next;
    struct list* prev;
};

struct list*
list_init(struct list* list);

struct list*
list_init_head(struct list* head);

bool
list_is_empty(const struct list* head);

void
list_enqueue_before(struct list* list, struct list* newlist);

void
list_enqueue_after(struct list* list, struct list* newlist);

void
list_enqueue_front(struct list* head, struct list* newlist);

void
list_enqueue_back(struct list* head, struct list* newlist);

void
list_dequeue(struct list* list);

struct list*
list_begin(const struct list* head);

const struct list*
list_end(const struct list* head);

struct list*
list_first(const struct list* head);

struct list*
list_last(const struct list* head);

struct list*
list_next(const struct list* list);

struct list*
list_prev(const struct list* list);
