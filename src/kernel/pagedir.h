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
        volatile pde_type entry[1024]; /* page-directory entries */
} __attribute__ (( aligned(4096) ));

int
page_directory_init(struct page_directory *pd);

void
page_directory_uninit(struct page_directory *pd);

int
page_directory_install_page_table(struct page_directory *pd,
                                  unsigned long pfindex,
                                  unsigned long index,
                                  unsigned int flags);

int
page_directory_install_page_tables(struct page_directory *pd,
                                   unsigned long pfindex,
                                   unsigned long index,
                                   unsigned long count,
                                   unsigned int flags);

int
page_directory_uninstall_page_table(struct page_directory *pd,
                                    unsigned long index);

int
page_directory_uninstall_page_tables(struct page_directory *pd,
                                     unsigned long index,
                                     unsigned long count);

