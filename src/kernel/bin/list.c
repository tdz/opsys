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
list_init_item(struct list* item)
{
    item->next = NULL;
    item->prev = NULL;

    return item;
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
list_enqueue_before(struct list* item, struct list* newitem)
{
    newitem->prev = item->prev;
    newitem->next = item;

    if (item->prev) {
        item->prev->next = newitem;
    }

    item->prev = newitem;
}

void
list_enqueue_after(struct list* item, struct list* newitem)
{
    newitem->prev = item;
    newitem->next = item->next;

    if (item->next) {
        item->next->prev = newitem;
    }

    item->next = newitem;
}

void
list_enqueue_front(struct list* head, struct list* newitem)
{
    list_enqueue_after(head, newitem);
}

void
list_enqueue_back(struct list* head, struct list* newitem)
{
    list_enqueue_before(head, newitem);
}

void
list_enqueue_sorted(struct list* head, struct list* newitem,
                    int (*cmp)(struct list*, struct list*))
{
    struct list* item = list_begin(head);

    while (item != list_end(head)) {
        if (cmp(newitem, item) > 0) {
            break;
        }
        item = list_next(item);
    }

    list_enqueue_before(item, newitem);
}

void
list_dequeue(struct list* item)
{
    if (item->next) {
        item->next->prev = item->prev;
    }
    if (item->prev) {
        item->prev->next = item->next;
    }

    item->prev = NULL;
    item->next = NULL;
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
list_rbegin(const struct list* head)
{
    return head->prev;
}

const struct list*
list_rend(const struct list* head)
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
list_next(const struct list* item)
{
    return item->next;
}

struct list*
list_prev(const struct list* item)
{
    return item->prev;
}
