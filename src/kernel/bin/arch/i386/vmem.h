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
#include "semaphore.h"
#include "vmem_32.h"
#include "vmemarea.h"

struct vmem
{
        struct semaphore sem; /**< lock of address-space data structure */
        struct vmem_32   vmem_32;
};

/*
 * Public functions
 */

/**
 * \brief init vmem structure
 * \param[in] as address of vmem structure
 * \param[in] alloc_aligned function for allocating contiguous page-aligned memory
 * \param[in] unref_aligned function for releasing allocated memory
 * \return 0 if successful, or a negative error code otherwise
 */
int
vmem_init(struct vmem *vmem,
          void* (alloc_aligned)(size_t, void*),
          void (*unref_aligned)(void*, size_t, void*), void* alloc_data);

int
vmem_init_from_parent(struct vmem* vmem, struct vmem* parent);

void
vmem_uninit(struct vmem *vmem);

int
vmem_alloc_frames(struct vmem *vmem, os_index_t pfindex,
                                                   os_index_t pgindex,
                                                   size_t pgcount,
                                                   unsigned int flags);

os_index_t
vmem_lookup_frame(struct vmem *vmem, os_index_t pgindex);

os_index_t
vmem_alloc_pages_at(struct vmem *vmem, os_index_t pg_index, size_t pg_count,
                    unsigned int flags);

os_index_t
vmem_alloc_pages_within(struct vmem *vmem, os_index_t pg_index_min,
                        os_index_t pg_index_max, size_t pg_count,
                        unsigned int flags);

os_index_t
vmem_alloc_pages_in_area(struct vmem* vmem,
                         enum vmem_area_name areaname,
                         size_t pgcount,
                         unsigned int flags);

int
vmem_map_pages_at(struct vmem *dst_as, os_index_t dst_pgindex,
                  struct vmem *src_as, os_index_t src_pgindex,
                  size_t pgcount, unsigned long pteflags);

os_index_t
vmem_map_pages_within(struct vmem *dst_as, os_index_t pg_index_min,
                      os_index_t pg_index_max, struct vmem *src_as,
                      os_index_t src_pgindex, size_t pgcount,
                      unsigned long dst_pteflags);

os_index_t
vmem_map_pages_in_area(struct vmem *dst_vmem,
                       enum vmem_area_name dst_areaname,
                       struct vmem *src_vmem,
                       os_index_t src_pgindex, size_t pgcount,
                       unsigned long dst_pteflags);

os_index_t
vmem_empty_pages_within(struct vmem *vmem, os_index_t pg_index_min,
                        os_index_t pg_index_max, size_t pgcount);

os_index_t
vmem_empty_pages_in_area(struct vmem* vmem,
                         enum vmem_area_name areaname, size_t pgcount);

/*
 * Public functions for Protected Mode setup
 */

int
vmem_init_nopg(struct vmem* vmem);

int
vmem_map_pageframes_nopg(struct vmem* vmem, os_index_t pfindex,
                         os_index_t pgindex, size_t count, unsigned int flags);

int
vmem_alloc_page_tables_nopg(struct vmem* vmem, os_index_t ptindex,
                            size_t ptcount, unsigned int flags);

/**
 * \brief Install temporary-mappings hack
 * \param[in] as Address of vmem structure
 * \return 0 if successful; a negative error code otherwise
 *
 * The lowest page table in kernel memory resides in highest page frame of
 * identity-mapped low memory. This allows for temporary mappings by writing
 * to low-area page.
 */
int
vmem_install_tmp_nopg(struct vmem* vmem);

/**
 * \brief Map paging structures into virtual address space
 * \param[in] The virtual address space
 */
int
vmem_map_paging_structures_nopg(struct vmem* vmem);

/**
 * \brief Enables paging
 * \param[in] vmem The current address space
 *
 * This is always the final _nopg function to be called. After it
 * returned, paging is enabled and VMEM callers should use the regular
 * interface.
 */
void
vmem_enable_paging_nopg(struct vmem* vmem);

/*
 * Fault handlers
 */

void
vmem_segfault_handler(void *ip);

void
vmem_pagefault_handler(void *ip, void *addr, unsigned long errcode);
