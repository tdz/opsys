/*
 *  opsys - A small, experimental operating system
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

enum vmem_area_name
{
        VIRTMEM_AREA_SYSTEM     = 0,
        VIRTMEM_AREA_LOW        = 1,
        VIRTMEM_AREA_USER       = 2,
        VIRTMEM_AREA_KERNEL_TMP = 3,
        VIRTMEM_AREA_KERNEL     = 4,
        LAST_VIRTMEM_AREA       = 5
};

enum vmem_area_flags
{
        VIRTMEM_AREA_FLAG_EMPTY      = 0,
        VIRTMEM_AREA_FLAG_KERNEL     = 1<<0,
        VIRTMEM_AREA_FLAG_POLUTE     = 1<<1,
        VIRTMEM_AREA_FLAG_IDENTITY   = 1<<2,
        VIRTMEM_AREA_FLAG_PAGETABLES = 1<<3,
        VIRTMEM_AREA_FLAG_GLOBAL     = 1<<4,
        VIRTMEM_AREA_FLAG_USER       = 1<<5
};

struct vmem_area
{
        os_index_t   pgindex;
        size_t       npages;
        unsigned int flags;
};

const struct vmem_area *
vmem_area_get_by_name(enum vmem_area_name name);

const struct vmem_area *
vmem_area_get_by_page(os_index_t pgindex);

int
vmem_area_contains_page(const struct vmem_area *area,
                           os_index_t pgindex);

