/*
 *  oskernel - A small experimental operating-system kernel
 *  Copyright (C) 2009  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#ifndef PAGEDIR_H
#define PAGEDIR_H

enum {
        PAGEDIR_SHIFT = 22,
        PAGEDIR_SIZE  = 1<<PAGEDIR_SHIFT,
        PAGEDIR_MASK  = PAGEDIR_SIZE-1
};

static __inline__ unsigned long
pagedir_index(unsigned long addr)
{
        return addr>>PAGEDIR_SHIFT;
}

static __inline__ unsigned long
pagedir_offset(unsigned long index)
{
        return index<<PAGEDIR_SHIFT;
}

static __inline__ unsigned long
pagedir_count(unsigned long bytes)
{
        return bytes ? 1+((bytes)>>PAGEDIR_SHIFT) : 0;
}

static __inline__ unsigned long
pagedir_floor(unsigned long addr)
{
        return addr & ~PAGEDIR_MASK;
}

static __inline__ unsigned long
pagedir_ceil(unsigned long addr)
{
        return ((addr>>PAGEDIR_SHIFT)+1) << PAGEDIR_SHIFT;
}

#endif

