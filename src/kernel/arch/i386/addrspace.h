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

struct page_directory;

/**
 * \brief Paging mode of the address space
 */
enum paging_mode
{
        PAGING_32BIT, /**< Address space uses 32-bit paging */
        PAGING_PAE /**< Address space uses PAE (currently not supported) */
};

struct address_space
{
        enum paging_mode       mode;
        union {
                struct page_directory *pd;
        } tlps;
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
                   enum paging_mode mode,
                   void *tlps);

void
address_space_uninit(struct address_space *as);


os_index_t
address_space_find_empty_pages(const struct page_directory *pd,
                               size_t npages,
                               os_index_t pgindex_beg,
                               os_index_t pgindex_end);

size_t
address_space_check_empty_pages(const struct page_directory *pd,
                                os_index_t pgindex,
                                size_t pgcount);

int
address_space_alloc_page_frames(struct page_directory *pd,
                                os_index_t pfindex,
                                os_index_t pgindex,
                                size_t pgcount,
                                unsigned int flags);

os_index_t
address_space_alloc_pages(struct page_directory *pd,
                          os_index_t pgindex,
                          size_t pgcount,
                          unsigned int flags);

os_index_t
address_space_lookup_pageframe(const struct page_directory *pd,
                               os_index_t pgindex);

int
address_space_flat_copy_areas(const struct page_directory *pd,
                              struct page_directory *dst,
                              unsigned long flags);

int
address_space_map_pages(const struct page_directory *src,
                        os_index_t src_pgindex,
                        size_t pgcount,
                        struct page_directory *dst,
                        os_index_t dst_pgindex,
                        unsigned long flags);

