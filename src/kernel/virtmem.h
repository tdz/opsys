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

enum virtmem_area_name;
struct vmem;

int
virtmem_alloc_frames(struct vmem *as, os_index_t pfindex,
                                                   os_index_t pgindex,
                                                   size_t pgcount,
                                                   unsigned int flags);

os_index_t
virtmem_lookup_frame(struct vmem *as, os_index_t pgindex);

os_index_t
virtmem_alloc_pages_at(struct vmem *as, os_index_t pg_index, size_t pg_count,
                       unsigned int flags);

os_index_t
virtmem_alloc_pages_within(struct vmem *as, os_index_t pg_index_min,
                           os_index_t pg_index_max, size_t pg_count,
                           unsigned int flags);

os_index_t
virtmem_alloc_pages_in_area(struct vmem *as,
                            enum virtmem_area_name areaname,
                            size_t pgcount,
                            unsigned int flags);

int
virtmem_map_pages_at(struct vmem *dst_as, os_index_t dst_pgindex,
                     struct vmem *src_as, os_index_t src_pgindex,
                     size_t pgcount, unsigned long pteflags);

os_index_t
virtmem_map_pages_within(struct vmem *dst_as, os_index_t pg_index_min,
                         os_index_t pg_index_max, struct vmem *src_as,
                         os_index_t src_pgindex, size_t pgcount,
                         unsigned long dst_pteflags);

os_index_t
virtmem_map_pages_in_area(struct vmem *dst_as,
                          enum virtmem_area_name dst_areaname,
                          struct vmem *src_as,
                          os_index_t src_pgindex,
                          size_t pgcount,
                          unsigned long dst_pteflags);

void
virtmem_segfault_handler(void *ip);

void
virtmem_pagefault_handler(void *ip, void *addr, unsigned long errcode);

