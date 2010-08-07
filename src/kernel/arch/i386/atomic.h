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

static __inline__ unsigned long
atomic_xchg(volatile void *addr, unsigned long new_value)
{
        unsigned long old_value;

        __asm__("lock xchg (%1), %%eax\n\t"
                : "=a"(old_value)
                : "r"(addr),
                  "a"(new_value)
                );

        return old_value;
}

static __inline__ void
atomic_inc(void *addr)
{
        __asm__("lock inc (%0)\n\t"
                :
                : "r"((unsigned long)addr)
                );
}

static __inline__ void
atomic_dec(void *addr)
{
        __asm__("lock dec (%0)\n\t"
                :
                : "r"((unsigned long)addr)
                );
}

