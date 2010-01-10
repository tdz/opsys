/*
 *  oskernel - A small experimental operating-system kernel
 *  Copyright (C) 2009-2010  Thomas Zimmermann <tdz@users.sourceforge.net>
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

enum virtmem_area_name {
        VIRTMEM_AREA_LOW       = 0,
        VIRTMEM_AREA_USER      = 1,
        VIRTMEM_AREA_TASKSTATE = 2,
        VIRTMEM_AREA_KERNEL    = 3
};

struct virtmem_area
{
        unsigned long pgindex;
        unsigned long npages;
};

const struct virtmem_area g_virtmem_area[4];

int
virtmem_install_kernel_area_low(struct page_directory *pd);

unsigned long
virtmem_lookup_physical_page(const struct page_directory *pt,
                             unsigned long virt_pgindex);

#endif

