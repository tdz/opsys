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

struct vmem;

int
vmem_32_map_pageframe_nopg(void *tlps, os_index_t pfindex,
                                       os_index_t pgindex, unsigned int flags);

int
vmem_32_alloc_page_table_nopg(void *tlps, os_index_t ptindex,
                              unsigned int flags);

int
vmem_32_install_tmp_nopg(void *tlps);

void
vmem_32_enable(const void *tlps);

int
vmem_32_alloc_frames(void *tlps, os_index_t pfindex, os_index_t pgindex,
                     size_t pgcount, unsigned int pteflags);

os_index_t
vmem_32_lookup_frame(const void *tlps, os_index_t pgindex);

size_t
vmem_32_check_empty_pages(const void *tlps, os_index_t pgindex,
                          size_t pgcount);

int
vmem_32_alloc_pages(void *tlps, os_index_t pgindex,
                                size_t pgcount, unsigned int pteflags);

int
vmem_32_map_pages(void *dst_tlps, os_index_t dst_pgindex,
                                  const struct vmem *src_as,
                                  os_index_t src_pgindex,
                                  size_t pgcount, unsigned long pteflags);

int
vmem_32_share_2nd_lvl_ps(void *dst_tlps, const void *src_tlps,
                         os_index_t pgindex, size_t pgcount);
