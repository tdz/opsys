/*
 * opsys - A small, experimental operating system
 * Copyright (C) 2016  Thomas Zimmermann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "irq.h"
#include <stddef.h>
#include <string.h>
#include "drivers/i8259/pic.h"
#include "idt.h"
#include "interupt.h"

static struct irq_handler*
irq_handler_of_list(struct list* item)
{
    return containerof(item, struct irq_handler, irqh);
}

void
irq_handler_init(struct irq_handler* irqh,
                 enum irq_status (*func)(unsigned char, struct irq_handler*))
{
    list_init_item(&irqh->irqh);
    irqh->func = func;
}

struct irq_handling {
    /* \attention Do not write with interupts enabled! */
    struct list irqh[256];
};

static struct irq_handling g_irq_handling;

void
init_irq_handling()
{
    struct list* head = g_irq_handling.irqh;
    const struct list* head_end = head + ARRAY_NELEMS(g_irq_handling.irqh);

    for (; head < head_end; ++head) {
        list_init_head(head);
    }
}

static void
irq_handler(unsigned char irqno)
{
    const struct list* head = g_irq_handling.irqh + irqno;
    struct list* item = list_begin(head);

    while (item != list_end(head)) {

        struct list* next = list_next(item);

        struct irq_handler* irqh = irq_handler_of_list(item);

        enum irq_status status = irqh->func(irqno, irqh);
        if (status == IRQ_HANDLED) {
            break;
        }

        /* don't touch *item after func() returned */
        item = next;
    }
}

int
install_irq_handler(unsigned char irqno, struct irq_handler* irqh)
{
    bool ints_on = cli_if_on();

    struct list* head = g_irq_handling.irqh + irqno;

    if (list_is_empty(head)) {
        /* We're handling a new interrupt, install an
         * interupt handler. */
        idt_install_irq_handler(irqno, irq_handler);
    }

    list_enqueue_back(head, &irqh->irqh);

    sti_if_on(ints_on);

    return 0;
}

void
remove_irq_handler(unsigned char irqno, struct irq_handler* irqh)
{
    bool ints_on = cli_if_on();

    list_dequeue(&irqh->irqh);

    if (list_is_empty(g_irq_handling.irqh + irqno)) {
        /* Remove interupt if we don't handle it
         * any longer. */
        idt_install_irq_handler(irqno, NULL);
    }

    sti_if_on(ints_on);
}
