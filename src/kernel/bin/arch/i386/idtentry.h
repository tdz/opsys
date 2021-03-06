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

enum {
        IDT_ENTRY_FLAG_SEGINMEM = 0x80,
        IDT_ENTRY_FLAG_32BITINT = 0x08,
        IDT_ENTRY_FLAG_16BITINT = 0x00,
        IDT_ENTRY_FLAG_TASKGATE = 0x05,
        IDT_ENTRY_FLAG_INTGATE  = 0x06,
        IDT_ENTRY_FLAG_TRAPGATE = 0x07
};

struct idt_entry
{
        unsigned short base_low;
        unsigned short tss;
        unsigned char  reserved;
        unsigned char  flags;
        unsigned short base_high;
};

void
idt_entry_init(struct idt_entry *idte, unsigned long  func,
                                       unsigned short tss,
                                       unsigned char  ring,
                                       unsigned char  flags);
