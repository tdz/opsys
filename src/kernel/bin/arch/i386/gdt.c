/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009  Thomas Zimmermann
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

#include "gdt.h"

enum
{
        GDT_FLAG_LOWGRAN = 0x80,
        GDT_FLAG_32BITSEG = 0x40,
        /*
         * flag bits
         */
        GDT_FLAG_SEGINMEM = 0x80,
        GDT_FLAG_CODEDATA = 0x10,
        GDT_FLAG_CODESEG = 0x08,
        GDT_FLAG_DATASEG = 0x00,
        GDT_FLAG_RDWRRDEX = 0x02
};

struct gdt_entry
{
        unsigned short limit_low;
        unsigned short base_low;
        unsigned char base_mid;
        unsigned char flags;
        unsigned char limit_high;
        unsigned char base_high;
};

static void
gdt_entry_init(struct gdt_entry *gdte, unsigned long base,
               unsigned long limit, unsigned char flags)
{
        gdte->limit_low = limit & 0xffff;
        gdte->base_low = base & 0xffff;
        gdte->base_mid = (base >> 16) & 0xff;
        gdte->flags = flags;
        gdte->limit_high = ((limit >> 16) & 0x0f) | GDT_FLAG_LOWGRAN |
                GDT_FLAG_32BITSEG;
        gdte->base_high = (base >> 24) & 0xff;
}

static struct gdt_entry g_gdt[3];

void
gdt_init()
{
        gdt_entry_init(g_gdt + 0, 0, 0, 0);
        gdt_entry_init(g_gdt + 1, 0x00000000,
                       0x00ffffff,
                       GDT_FLAG_SEGINMEM |
                       GDT_FLAG_CODEDATA |
                       GDT_FLAG_CODESEG | GDT_FLAG_RDWRRDEX);
        gdt_entry_init(g_gdt + 2, 0x00000000,
                       0x00ffffff,
                       GDT_FLAG_SEGINMEM |
                       GDT_FLAG_CODEDATA |
                       GDT_FLAG_DATASEG | GDT_FLAG_RDWRRDEX);
}

struct gdt_register
{
        unsigned short limit __attribute__ ((packed));
        unsigned long base __attribute__ ((packed));
};

static const struct gdt_register gdtr __asm__("_gdtr") __attribute__((unused)) =
{
    .limit = sizeof(g_gdt),
    .base = (unsigned long)g_gdt,
};

void
gdt_install()
{
        __asm__("lgdt (_gdtr)\n\t");
}
