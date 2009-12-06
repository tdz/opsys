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

#ifndef PHYSPAGE_H
#define PHYSPAGE_H

enum {
        PHYSPAGE_SHIFT = 12,
        PHYSPAGE_SIZE  = 1<<PHYSPAGE_SHIFT,
        PHYSPAGE_MASK  = PHYSPAGE_SIZE-1
};

static inline unsigned long
physpage_index(unsigned long addr)
{
        return addr>>PHYSPAGE_SHIFT;
}

static inline unsigned long
physpage_offset(unsigned long index)
{
        return index<<PHYSPAGE_SHIFT;
}

static inline unsigned long
physpage_count(unsigned long count)
{
        return count ? 1+((count)>>PHYSPAGE_SHIFT) : 0;
}

static inline unsigned long
physpage_floor(unsigned long addr)
{
        return addr & ~PHYSPAGE_MASK;
}

static inline unsigned long
physpage_ceil(unsigned long addr)
{
        return ((addr>>PHYSPAGE_SHIFT)+1) << PHYSPAGE_SHIFT;
}

#endif

