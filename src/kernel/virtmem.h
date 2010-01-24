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
        VIRTMEM_AREA_KERNEL_TMP = 2,
        VIRTMEM_AREA_KERNEL     = 3,
        LAST_VIRTMEM_AREA       = 4
};

enum virtmem_area_flags {
        VIRTMEM_AREA_FLAG_EMPTY      = 0,
        VIRTMEM_AREA_FLAG_KERNEL     = 1<<0,
        VIRTMEM_AREA_FLAG_POLUTE     = 1<<1,
        VIRTMEM_AREA_FLAG_IDENTITY   = 1<<2,
        VIRTMEM_AREA_FLAG_PAGETABLES = 1<<3,
        VIRTMEM_AREA_FLAG_GLOBAL     = 1<<4,
        VIRTMEM_AREA_FLAG_USER       = 1<<5
};

int
virtmem_init(struct page_directory *pd);

int
virtmem_alloc_page_frames(struct page_directory *pd, unsigned long pfindex,
                                                     unsigned long pgindex,
                                                     unsigned long pgcount,
                                                     unsigned int flags);

int
virtmem_alloc_pages(struct page_directory *pd, unsigned long pgindex,
                                               unsigned long pgcount,
                                               unsigned int flags);

os_index_t
virtmem_alloc_pages_in_area(struct page_directory *pd,
                            unsigned long npages,
                            enum virtmem_area_name areaname,
                            unsigned int flags);

int
virtmem_flat_copy_areas(const struct page_directory *pd,
                              struct page_directory *dst,
                              unsigned long flags);

os_index_t
virtmem_lookup_pageframe(const struct page_directory *pt, os_index_t pgindex);

void
virtmem_segfault_handler(void *ip);

void
virtmem_pagefault_handler(void *ip, void *addr);

