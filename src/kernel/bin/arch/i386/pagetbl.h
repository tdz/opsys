/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009-2010  Thomas Zimmermann
 *  Copyright (C) 2016-2017  Thomas Zimmermann
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

#include <arch/i386/page.h>
#include "pte.h"

enum {
        PAGETABLE_SHIFT = 22,
        PAGETABLE_SIZE  = 1<<PAGETABLE_SHIFT,
        PAGETABLE_MASK  = PAGETABLE_SIZE-1
};

static __inline__ os_index_t
pagetable_index(const void *addr)
{
        return (((size_t)addr)>>PAGETABLE_SHIFT);
}

static __inline__ size_t
pagetable_offset(os_index_t index)
{
        return index<<PAGETABLE_SHIFT;
}

static __inline__ void *
pagetable_address(os_index_t ptindex)
{
        return (void*)pagetable_offset(ptindex);
}

static __inline__ size_t
pagetable_count(const void *addr, size_t bytes)
{
        return bytes ? 1+pagetable_index(addr+bytes-1)-pagetable_index(addr)
                     : 0;
}

static __inline__ void *
pagetable_floor(const void *addr)
{
        return (void*)(((size_t)addr) & ~PAGETABLE_MASK);
}

static __inline__ void *
pagetable_ceil(const void *addr)
{
        return (void*)(((((size_t)addr)>>PAGETABLE_SHIFT)+1)<<PAGETABLE_SHIFT);
}

static __inline__ size_t
pagetable_page_index(os_index_t pgindex)
{
        return pgindex&0x3ff;
}

struct page_table
{
        volatile pte_type entry[1024]; /* page-table entries */
};

int
page_table_init(struct page_table *pt);

void
page_table_uninit(struct page_table *pt);

int
page_table_map_page_frame(struct page_table *pt,
                          os_index_t pfindex,
                          os_index_t index,
                          unsigned int flags);

int
page_table_map_page_frames(struct page_table *pt,
                           os_index_t pfindex,
                           os_index_t index,
                           size_t count,
                           unsigned int flags);

int
page_table_unmap_page_frame(struct page_table *pt, os_index_t index);

int
page_table_unmap_page_frames(struct page_table *pt, os_index_t index,
                                                    size_t count);
