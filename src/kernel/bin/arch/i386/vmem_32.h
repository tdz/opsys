/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2010  Thomas Zimmermann
 *  Copyright (C) 2016  Thomas Zimmermann
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

struct page_directory;

struct vmem_32 {
    struct page_directory* pd;
};

int
vmem_32_init(struct vmem_32* vmem32,
             void* (alloc_aligned)(size_t, void*),
             void (*unref_aligned)(void*, size_t, void*), void* alloc_data);

void
vmem_32_uninit(struct vmem_32* vmem32);

int
vmem_32_alloc_frames(struct vmem_32* vmem32,
                     os_index_t pfindex, os_index_t pgindex, size_t pgcount,
                     unsigned int pteflags);

os_index_t
vmem_32_lookup_frame(struct vmem_32* vmem32, os_index_t pgindex);

size_t
vmem_32_check_empty_pages(struct vmem_32* vmem32, os_index_t pgindex,
                          size_t pgcount);

int
vmem_32_alloc_pages(struct vmem_32* vmem32, os_index_t pgindex,
                    size_t pgcount, unsigned int pteflags);

int
vmem_32_map_pages(struct vmem_32* dst_as, os_index_t dst_pgindex,
                  struct vmem_32* src_as, os_index_t src_pgindex,
                  size_t pgcount, unsigned long pteflags);

int
vmem_32_share_page_range(struct vmem_32* dst_vmem32,
                         struct vmem_32* src_vmem32,
                         os_index_t pgindex, size_t pgcount);

/*
 * Public functions for Protected Mode setup
 */

int
vmem_32_map_pageframe_nopg(struct vmem_32* vmem32,
                           os_index_t pfindex,
                           os_index_t pgindex, unsigned int flags);

int
vmem_32_alloc_page_table_nopg(struct vmem_32* vmem32,
                              os_index_t ptindex, unsigned int flags);

int
vmem_32_install_tmp_nopg(struct vmem_32* vmem32);

int
vmem_32_map_paging_structures_nopg(struct vmem_32* vmem32);

void
vmem_32_enable_paging_nopg(struct vmem_32* vmem32);
