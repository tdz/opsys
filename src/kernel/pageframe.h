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

enum {
        PAGEFRAME_SHIFT = 12,
        PAGEFRAME_SIZE  = 1<<PAGEFRAME_SHIFT,
        PAGEFRAME_MASK  = PAGEFRAME_SIZE-1
};

static __inline__ os_index_t
pageframe_index(const void *addr)
{
        return ((size_t)addr)>>PAGEFRAME_SHIFT;
}

static __inline__ size_t
pageframe_offset(os_index_t index)
{
        return index<<PAGEFRAME_SHIFT;
}

static __inline__ void*
pageframe_address(os_index_t index)
{
        return (void*)pageframe_offset(index);
}

static __inline__ size_t
pageframe_count(size_t bytes)
{
        return (bytes+PAGEFRAME_SIZE-1)>>PAGEFRAME_SHIFT;
}

static __inline__ size_t
pageframe_span(const void *addr, size_t bytes)
{
        return bytes ? 1+pageframe_index(addr+bytes-1)-pageframe_index(addr)
                     : 0;
}

static __inline__ size_t
pageframe_memory(os_index_t nframes)
{
        return nframes*PAGEFRAME_SIZE;
}

static __inline__ void*
pageframe_floor(const void *addr)
{
        return (void*)(((size_t)addr) & ~PAGEFRAME_MASK);
}

static __inline__ void*
pageframe_ceil(const void *addr)
{
        return (void*)(((((size_t)addr)>>PAGEFRAME_SHIFT)+1)<<PAGEFRAME_SHIFT);
}

