/*
 *  oskernel - A small experimental operating-system kernel
 *  Copyright (C) 2009  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#ifndef VIRTMEM_H
#define VIRTMEM_H

const unsigned long g_min_kernel_virtaddr;
const unsigned long g_max_kernel_virtaddr;
const unsigned long g_max_user_virtaddr;

struct page_table
{
        pt_entry entry[1024];
};

struct page_table *
page_table_create(void);

void
page_table_destroy(struct page_table *pt);

struct page_directory
{
        pd_entry      pentry[1024]; /* physical addresses */
        unsigned long ventry[1024]; /* virtual addresses */
};

struct page_directory *
page_directory_create(void);

void
page_directory_destroy(struct page_directory *pd);

unsigned long
page_directory_alloc_pages(struct page_directory *pt, unsigned long npages);

unsigned long
page_directory_alloc_phys_pages(struct page_directory *pt,
                                unsigned long phys_pgindex,
                                unsigned long npages);

unsigned long
page_directory_alloc_phys_pages_at(struct page_directory *pt,
                                   unsigned long virt_pgindex,
                                   unsigned long phys_pgindex,
                                   unsigned long npages);

void
page_directory_release_pages(struct page_directory *pt, unsigned long pgindex,
                                                        unsigned npages);

unsigned long
page_directory_lookup_phys(const struct page_directory *pt,
                           unsigned long virtaddr);

int
page_directory_install_page_tables(struct page_directory *pd,
                                   unsigned long virt_pgindex,
                                   unsigned long npages);

#endif

