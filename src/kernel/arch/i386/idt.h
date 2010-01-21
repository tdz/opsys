/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009-2010  Thomas Zimmermann <tdz@users.sourceforge.net>
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

void
idt_init(void);

void
idt_install(void);

void
idt_install_irq(void);

int
idt_install_handler(unsigned char intno, address_type handler);

int
idt_install_invalid_opcode_handler(void (*hdlr)(address_type));

int
idt_install_segfault_handler(void (*hdlr)(address_type));

int
idt_install_pagefault_handler(void (*hdlr)(address_type, address_type));

int
idt_install_irq_handler(unsigned char irqno, void (*hdlr)(unsigned char));

