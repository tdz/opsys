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

static __inline__ void
hlt(void)
{
        __asm__("hlt\n\t");
}

static __inline__ unsigned long
eflags(void)
{
        unsigned long eflags;

        __asm__("pushf\n\t"
                "movl (%%esp), %0\n\t"
                "popf\n\t"
                        : "=r"(eflags));

        return eflags;
}

static __inline__ unsigned long
cr0(void)
{
        unsigned long cr0;

        __asm__("movl %%cr0, %0\n\t"
                        : "=r"(cr0));

        return cr0;
}

static __inline__ unsigned long
cr2(void)
{
        unsigned long cr2;

        __asm__("movl %%cr2, %0\n\t"
                        : "=r"(cr2));

        return cr2;
}

static __inline__ unsigned long
cr3(void)
{
        unsigned long cr3;

        __asm__("movl %%cr3, %0\n\t"
                        : "=r"(cr3));

        return cr3;
}

static __inline__ unsigned long
cr4(void)
{
        unsigned long cr4;

        __asm__("movl %%cr4, %0\n\t"
                        : "=r"(cr4));

        return cr4;
}

