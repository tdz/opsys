/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009-2010  Thomas Zimmermann
 *  Copyright (C) 2016       Thomas Zimmermann
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

#include <sys/types.h>

enum {
        PAGE_SHIFT = 12,
        PAGE_SIZE  = 1<<PAGE_SHIFT,
        PAGE_MASK  = PAGE_SIZE-1
};

static __inline__ os_index_t
page_index(const void *addr)
{
        return ((size_t)addr)>>PAGE_SHIFT;
}

static __inline__ size_t
page_offset(os_index_t pgindex)
{
        return pgindex<<PAGE_SHIFT;
}

static __inline__ void *
page_address(os_index_t pgindex)
{
        return (void*)page_offset(pgindex);
}

static __inline__ size_t
page_count(const void *addr, size_t bytes)
{
        return bytes ? 1 + page_index(addr+bytes-1) - page_index(addr)
                     : 0;
}

static __inline__ size_t
page_memory(size_t npages)
{
        return npages*PAGE_SIZE;
}

static __inline__ void *
page_floor(const void *addr)
{
        return (void*)(((size_t)addr) & ~PAGE_MASK);
}

static __inline__ void *
page_ceil(const void *addr)
{
        return (void*)(((((size_t)addr)>>PAGE_SHIFT)+1) << PAGE_SHIFT);
}
