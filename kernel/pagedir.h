/*
 *  oskernel - A small experimental operating-system kernel
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

struct page_directory
{
        pde_type      pentry[1024]; /* page-directory entries */
        unsigned long ventry[1024]; /* TODO: reimpl routines that need this, then remove */
};

int
page_directory_init(struct page_directory *pd);

void
page_directory_uninit(struct page_directory *pd);

int
page_directory_alloc_page_table_at(struct page_directory *pd,
                                   unsigned long ptindex,
                                   unsigned int flags);

int
page_directory_alloc_page_tables_at(struct page_directory *pd,
                                    unsigned long ptindex,
                                    unsigned long ptcount,
                                    unsigned int flags);

int
page_directory_map_pageframe_at(struct page_directory *pd,
                                unsigned long pfindex,
                                unsigned long pgindex,
                                unsigned int flags);

int
page_directory_map_pageframes_at(struct page_directory *pd,
                                 unsigned long pfindex,
                                 unsigned long pgindex,
                                 unsigned long count,
                                 unsigned int flags);

int
page_directory_unmap_page(struct page_directory *pd,
                          unsigned long pgindex);

int
page_directory_unmap_pages(struct page_directory *pd,
                           unsigned long pgindex,
                           unsigned long pgcount);

int
page_directory_install_page_tables_at(struct page_directory *pd,
                                      unsigned long pgindex_tgt,
                                      unsigned long ptindex,
                                      unsigned long ptcount,
                                      unsigned int flags);

unsigned long
page_directory_check_empty_pages_at(const struct page_directory *pd,
                                    unsigned long pgindex,
                                    unsigned long npages);

unsigned long
page_directory_find_empty_pages(const struct page_directory *pd,
                                unsigned long npages,
                                unsigned long pgindex,
                                unsigned long pgcount);

int
page_directory_alloc_pages_at(struct page_directory *pd,
                              unsigned long pgindex,
                              unsigned long pgcount,
                              unsigned int pteflags);

