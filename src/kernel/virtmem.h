/*
 *  opsys - A small, experimental operating system
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

enum virtmem_area_name {
        VIRTMEM_AREA_LOW        = 0,
        VIRTMEM_AREA_USER       = 1,
        VIRTMEM_AREA_TASKSTATE  = 2,
        VIRTMEM_AREA_KERNEL_TMP = 3,
        VIRTMEM_AREA_KERNEL     = 4,
        LAST_VIRTMEM_AREA       = 5
};

enum virtmem_area_flags {
        VIRTMEM_AREA_FLAG_EMPTY      = 0,
        VIRTMEM_AREA_FLAG_KERNEL     = 1<<0,
        VIRTMEM_AREA_FLAG_POLUTE     = 1<<1,
        VIRTMEM_AREA_FLAG_IDENTITY   = 1<<2,
        VIRTMEM_AREA_FLAG_PAGETABLES = 1<<3,
        VIRTMEM_AREA_FLAG_USER       = 1<<4
};

struct virtmem_area
{
        unsigned long pgindex;
        unsigned long npages;
        unsigned long flags;
};

const struct virtmem_area g_virtmem_area[LAST_VIRTMEM_AREA];

int
virtmem_install(struct page_directory *pd);

int
virtmem_alloc_page_frames(struct page_directory *pd, unsigned long pfindex,
                                                     unsigned long pgindex,
                                                     unsigned long pgcount,
                                                     unsigned int flags);

int
virtmem_alloc_pages(struct page_directory *pd, unsigned long pgindex,
                                               unsigned long pgcount,
                                               unsigned int flags);

unsigned long
virtmem_alloc_pages_in_area(struct page_directory *pd, unsigned long npages,
                      const struct virtmem_area *area,
                            unsigned int flags);

unsigned long
virtmem_lookup_physical_page(const struct page_directory *pt,
                             unsigned long virt_pgindex);

