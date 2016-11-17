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

struct list {
    struct list* next;
    struct list* prev;
};

struct list*
list_init(struct list* list);

struct list*
list_enqueue_before(struct list* list, struct list* newlist);

struct list*
list_enqueue_after(struct list* list, struct list* newlist);

struct list*
list_dequeue(struct list* list);

struct list*
list_next(const struct list* list);

struct list*
list_prev(const struct list* list);
