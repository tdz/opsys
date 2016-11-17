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
list_init_head(struct list* head)
{
    head->next = head;
    head->prev = head;

    return head;
}

bool
list_is_empty(const struct list* head)
{
    return list_begin(head) == list_end(head);
}

void
list_enqueue_before(struct list* list, struct list* newlist)
{
    newlist->prev = list->prev;
    newlist->next = list;

    if (list->prev) {
        list->prev->next = newlist;
    }

    list->prev = newlist;
}

void
list_enqueue_after(struct list* list, struct list* newlist)
{
    newlist->prev = list;
    newlist->next = list->next;

    if (list->next) {
        list->next->prev = newlist;
    }

    list->next = newlist;
}

void
list_enqueue_front(struct list* head, struct list* newlist)
{
    list_enqueue_after(head, newlist);
}

void
list_enqueue_back(struct list* head, struct list* newlist)
{
    list_enqueue_before(head, newlist);
}

void
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
}

struct list*
list_begin(const struct list* head)
{
    return head->next;
}

const struct list*
list_end(const struct list* head)
{
    return head;
}

struct list*
list_first(const struct list* head)
{
    if (list_is_empty(head)) {
        return NULL;
    }
    return list_begin(head);
}

struct list*
list_last(const struct list* head)
{
    if (list_is_empty(head)) {
        return NULL;
    }
    return list_prev(list_end(head));
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
