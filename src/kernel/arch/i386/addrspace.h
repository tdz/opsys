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

/**
 * \brief Paging mode of address space
 */
enum paging_mode {
        PAGING_32BIT, /**< Address space uses 32-bit paging */
        PAGING_PAE /**< Address space uses PAE (currently not supported) */
};

struct address_space
{
        spinlock_type    lock; /**< Lock of address-space data structure */
        enum paging_mode pgmode; /**< Address space's paging mode */
        void            *tlps; /**< Address of top-level paging structure */
};

/**
 * \brief Init address_space structure
 * \param[in] as Address of address_space structure
 * \param[in] mode Paging mode
 * \param[in] tps Address of top-level paging structure
 * \return 0 if successful; a negative error code otherwise
 */
int
address_space_init(struct address_space *as,
                   enum paging_mode pgmode,
                   void *tlps);

void
address_space_uninit(struct address_space *as);

/**
 * \brief Install temporary-mappings hack
 * \param[in] as Address of address_space structure
 * \return 0 if successful; a negative error code otherwise
 *
 * The lowest page table in kernel memory resides in highest page frame of
 * identity-mapped low memory. This allows for temporary mappings by writing
 * to low-area page.
 */
int
address_space_install_tmp(struct address_space *as);

void
address_space_enable(const struct address_space *as);

int
address_space_map_pageframes_nopg(struct address_space *as,
                                  os_index_t pfindex,
                                  os_index_t pgindex,
                                  size_t count,
                                  unsigned int flags);

int
address_space_alloc_page_tables_nopg(struct address_space *as,
                                     os_index_t ptindex,
                                     size_t ptcount,
                                     unsigned int flags);

os_index_t
address_space_find_empty_pages(const struct address_space *as,
                               size_t npages,
                               os_index_t pgindex_beg,
                               os_index_t pgindex_end);

size_t
address_space_check_empty_pages(const struct address_space *as,
                                os_index_t pgindex,
                                size_t pgcount);

int
address_space_alloc_pageframes(struct address_space *as,
                               os_index_t pfindex,
                               os_index_t pgindex,
                               size_t pgcount,
                               unsigned int flags);

os_index_t
address_space_alloc_pages(struct address_space *as,
                          os_index_t pgindex,
                          size_t pgcount,
                          unsigned int flags);

int
address_space_map_pages(const struct address_space *src_as,
                              os_index_t src_pgindex,
                              size_t pgcount,
                              struct address_space *dst_as,
                              os_index_t dst_pgindex,
                              unsigned long flags);

os_index_t
address_space_lookup_pageframe(const struct address_space *as,
                                     os_index_t pgindex);

int
address_space_share_2nd_lvl_ps(struct address_space *dst_as,
                         const struct address_space *src_as,
                               os_index_t pgindex,
                               size_t pgcount);

