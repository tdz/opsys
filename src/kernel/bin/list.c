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

#include "list.h"
#include <stddef.h>

struct list*
list_init(struct list* list)
{
    list->next = NULL;
    list->prev = NULL;

    return list;
}

struct list*
list_enqueue_before(struct list* list, struct list* newlist)
{
    if (!list) {
        return newlist;
    }

    newlist->prev = list->prev;
    newlist->next = list;

    if (list->prev) {
        list->prev->next = newlist;
    }

    list->prev = newlist;

    return newlist;
}

struct list*
list_enqueue_after(struct list* list, struct list* newlist)
{
    if (!list) {
        return newlist;
    }

    newlist->prev = list;
    newlist->next = list->next;

    if (list->next) {
        list->next->prev = newlist;
    }

    list->next = newlist;

    return list;
}

struct list*
list_dequeue(struct list* list)
{
    if (list->next) {
        list->next->prev = list->prev;
    }
    if (list->prev) {
        list->prev->next = list->next;
    }

    list->prev = NULL;
    list->next = NULL;

    return list;
}

struct list*
list_next(const struct list* list)
{
    return list->next;
}

struct list*
list_prev(const struct list* list)
{
    return list->prev;
}
