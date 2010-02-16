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

#include <stddef.h>
#include <sys/types.h>

#include <alloc.h>

#include "list.h"

static struct list *
list_alloc(void)
{
        return kmalloc(sizeof(struct list));
}

struct list *
list_init(struct list *list, struct list *prev, struct list *next, void *data)
{
        list->next = next;
        list->data = data;
        list->prev = prev;

        if (prev) {
                prev->next = list;
        }
        if (next) {
                next->prev = list;
        }

        return list;
}

struct list *
list_append(struct list *prev, void *data)
{
        struct list *list = list_alloc();

        if (!list) {
                goto err_list_alloc;
        }

        list_init(list, prev, NULL, data);

        return list;

err_list_alloc:
        return NULL;
}

struct list *
list_prepend(struct list *next, void *data)
{
        struct list *list = list_alloc();

        if (!list) {
                goto err_list_alloc;
        }

        list_init(list, NULL, next, data);

        return list;

err_list_alloc:
        return NULL;
}

void
list_free(struct list *list)
{
        return kfree(list);
}

struct list *
list_deque(struct list *list)
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

struct list *
list_next(const struct list *list)
{
        return list->next;
}

struct list *
list_prev(const struct list *list)
{
        return list->prev;
}

void *
list_data(const struct list *list)
{
        return list->data;
}

