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

#include "page.h"

enum vmem_area_name
{
        VMEM_AREA_SYSTEM     = 0, /**< the first page in the address space */
        VMEM_AREA_KERNEL_LOW = 1, /**< low kernel virtual memory */
        VMEM_AREA_USER       = 2, /**< user virtual memory */
        VMEM_AREA_TMPMAP     = 3, /**< high kernel virtual memory for temporary mappings */
        VMEM_AREA_KERNEL     = 4, /**< high kernel virtual memory */
        LAST_VMEM_AREA       = 5 /**< the index of the last entry in vmem_area_name */
};

enum vmem_area_flags
{
        VMEM_AREA_FLAG_EMPTY      = 0, /**< empty */
        VMEM_AREA_FLAG_KERNEL     = 1<<0, /**< kernel-reserved memory */
        VMEM_AREA_FLAG_POLUTE     = 1<<1, /**< map on startup */
        VMEM_AREA_FLAG_IDENTITY   = 1<<2, /**< map directly into kernel virtual address space */
        VMEM_AREA_FLAG_PAGETABLES = 1<<3, /**< memory for allocating page tables */
        VMEM_AREA_FLAG_GLOBAL     = 1<<4, /**< mapped into all virtual address spaces */
        VMEM_AREA_FLAG_USER       = 1<<5 /**< user memory */
};

struct vmem_area
{
        os_index_t   pgindex;
        size_t       npages;
        unsigned int flags;
};

const struct vmem_area*
vmem_area_get_by_name(enum vmem_area_name name);

const struct vmem_area*
vmem_area_get_by_page(os_index_t pgindex);

int
vmem_area_contains_page(const struct vmem_area* area, os_index_t pgindex);
