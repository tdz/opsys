/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2010  Thomas Zimmermann <tdz@users.sourceforge.net>
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

struct semaphore;

/**
 * \brief paging mode of address space
 */
enum paging_mode
{
        PAGING_32BIT, /**< address space uses 32-bit paging */
        PAGING_PAE /**< address space uses PAE (currently not supported) */
};

struct vmem
{
        struct semaphore sem; /**< lock of address-space data structure */
        enum paging_mode pgmode; /**< address space's paging mode */
        void            *tlps; /**< address of top-level paging structure */
};

int
vmem_map_pageframes_nopg(struct vmem *as, os_index_t pfindex,
                         os_index_t pgindex, size_t count, unsigned int flags);

int
vmem_alloc_page_tables_nopg(struct vmem *as, os_index_t ptindex,
                            size_t ptcount, unsigned int flags);

/**
 * \brief init vmem structure
 * \param[in] as address of vmem structure
 * \param[in] pgmode paging mode
 * \param[in] tlps address of top-level paging structure
 * \return 0 if successful, or a negative error code otherwise
 */
int
vmem_init(struct vmem *as, enum paging_mode pgmode, void *tlps);

void
vmem_uninit(struct vmem *as);

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
vmem_install_tmp(struct vmem *as);

void
vmem_enable(const struct vmem *as);

os_index_t
vmem_find_empty_pages(const struct vmem *as, size_t npages,
                      os_index_t pgindex_beg, os_index_t pgindex_end);

size_t
vmem_check_empty_pages(const struct vmem *as, os_index_t pgindex,
                       size_t pgcount);

int
vmem_alloc_frames(struct vmem *as, os_index_t pfindex, os_index_t pgindex,
                  size_t pgcount, unsigned int flags);

os_index_t
vmem_lookup_frame(const struct vmem *as, os_index_t pgindex);

int
vmem_alloc_pages(struct vmem *as, os_index_t pgindex, size_t pgcount,
                 unsigned int flags);

int
vmem_map_pages(struct vmem *dst_as, os_index_t dst_pgindex,
               const struct vmem *src_as, os_index_t src_pgindex,
               size_t pgcount, unsigned long flags);

int
vmem_share_2nd_lvl_ps(struct vmem *dst_as, const struct vmem *src_as,
                      os_index_t pgindex, size_t pgcount);

