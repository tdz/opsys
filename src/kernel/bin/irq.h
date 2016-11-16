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

#pragma once

#include "list.h"

enum irq_status {
    IRQ_HANDLED,
    IRQ_NOT_HANDLED
};

struct irq_handler {
    struct list irqh;
    enum irq_status (*func)(unsigned char, struct irq_handler*);
};

void
irq_handler_init(struct irq_handler* irqh,
                 enum irq_status (*func)(unsigned char, struct irq_handler*));

void
init_irq_handling(void);

int
install_irq_handler(unsigned char irqno, struct irq_handler* irqh);

void
remove_irq_handler(unsigned char irqno, struct irq_handler* irqh);
