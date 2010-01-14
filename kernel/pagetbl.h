/*
 *  oskernel - A small experimental operating-system kernel
 *  Copyright (C) 2009-2010  Thomas Zimmermann <tdz@users.sourceforge.net>
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
        PAGETABLE_SHIFT = 22,
        PAGETABLE_SIZE  = 1<<PAGETABLE_SHIFT,
        PAGETABLE_MASK  = PAGETABLE_SIZE-1
};

static __inline__ unsigned long
pagetable_index(unsigned long addr)
{
        return addr>>PAGETABLE_SHIFT;
}

static __inline__ unsigned long
pagetable_offset(unsigned long index)
{
        return index<<PAGETABLE_SHIFT;
}

static __inline__ void *
pagetable_address(unsigned long index)
{
        return (void*)pagetable_offset(index);
}

static __inline__ unsigned long
pagetable_count(unsigned long addr, unsigned long bytes)
{
        return bytes ? 1+pagetable_index(addr+bytes-1)-pagetable_index(addr)
                     : 0;
}

static __inline__ unsigned long
pagetable_floor(unsigned long addr)
{
        return addr & ~PAGETABLE_MASK;
}

static __inline__ unsigned long
pagetable_ceil(unsigned long addr)
{
        return ((addr>>PAGETABLE_SHIFT)+1) << PAGETABLE_SHIFT;
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
                          unsigned long pfindex,
                          unsigned long index,
                          int flags);

int
page_table_map_page_frames(struct page_table *pt,
                           unsigned long pfindex,
                           unsigned long index,
                           unsigned long count,
                           int flags);

int
page_table_unmap_page_frame(struct page_table *pt, unsigned long index);

int
page_table_unmap_page_frames(struct page_table *pt, unsigned long index,
                                                    unsigned long count);

