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

#include "vmemarea.h"

struct vmem;

int
vmem_helper_init_kernel_vmem(struct vmem *as);

int
vmem_helper_init_vmem_from_parent(struct vmem *parent, struct vmem *as);

int
vmem_helper_allocate_vmem_from_parent(struct vmem *parent, struct vmem **as);

os_index_t
vmem_helper_alloc_pages_in_area(struct vmem *as,
                            enum vmem_area_name areaname,
                            size_t pgcount,
                            unsigned int flags);

os_index_t
vmem_helper_map_pages_in_area(struct vmem *dst_as,
                          enum vmem_area_name dst_areaname,
                          struct vmem *src_as,
                          os_index_t src_pgindex,
                          size_t pgcount,
                          unsigned long dst_pteflags);

os_index_t
vmem_helper_empty_pages_in_area(struct vmem *vmem,
                                enum vmem_area_name areaname, size_t pgcount);
