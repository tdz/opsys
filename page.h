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

#ifndef PAGE_H
#define PAGE_H

enum {
        PAGE_SHIFT = 12,
        PAGE_SIZE  = 1<<PAGE_SHIFT,
        PAGE_MASK  = PAGE_SIZE-1
};

static __inline__ unsigned long
page_index(unsigned long addr)
{
        return addr>>PAGE_SHIFT;
}

static __inline__ unsigned long
page_offset(unsigned long index)
{
        return index<<PAGE_SHIFT;
}

#define PAGE_COUNT(_bytes) \
        (((_bytes)+(PAGE_SIZE-1))>>PAGE_SHIFT)

#define PAGE_MEMORY(_npages) \
        ((_npages)*PAGE_SIZE)

static __inline__ unsigned long
page_floor(unsigned long addr)
{
        return addr & ~PAGE_MASK;
}

static __inline__ unsigned long
page_ceil(unsigned long addr)
{
        return ((addr>>PAGE_SHIFT)+1) << PAGE_SHIFT;
}

#endif

