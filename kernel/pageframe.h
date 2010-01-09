/*
 *  oskernel - A small experimental operating-system kernel
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

#ifndef PAGEFRAME_H
#define PAGEFRAME_H

enum {
        PAGEFRAME_SHIFT = 12,
        PAGEFRAME_SIZE  = 1<<PAGEFRAME_SHIFT,
        PAGEFRAME_MASK  = PAGEFRAME_SIZE-1
};

static __inline__ unsigned long
pageframe_index(unsigned long addr)
{
        return addr>>PAGEFRAME_SHIFT;
}

static __inline__ unsigned long
pageframe_offset(unsigned long index)
{
        return index<<PAGEFRAME_SHIFT;
}

static __inline__ void *
pageframe_address(unsigned long index)
{
        return (void*)pageframe_offset(index);
}

static __inline__ unsigned long
pageframe_count(unsigned long bytes)
{
        return (bytes+PAGEFRAME_SIZE-1)>>PAGEFRAME_SHIFT;
}

static __inline__ unsigned long
pageframe_memory(unsigned long nframes)
{
        return nframes*PAGEFRAME_SIZE;
}

static __inline__ unsigned long
pageframe_floor(unsigned long addr)
{
        return addr & ~PAGEFRAME_MASK;
}

static __inline__ unsigned long
pageframe_ceil(unsigned long addr)
{
        return ((addr>>PAGEFRAME_SHIFT)+1) << PAGEFRAME_SHIFT;
}

#endif

