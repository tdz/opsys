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

#include <stddef.h>
#include <sys/types.h>

#include "vmemarea.h"

static const struct vmem_area g_vmem_area[LAST_VIRTMEM_AREA] = {
        {                       /* system: <4 KiB */
         .pgindex = 0,
         .npages = 1,
         .flags = 0},
        {                       /* low kernel virtual memory: <4 MiB */
         .pgindex = 1,
         .npages = 1023,
         .flags = VIRTMEM_AREA_FLAG_KERNEL |
         VIRTMEM_AREA_FLAG_IDENTITY |
         VIRTMEM_AREA_FLAG_POLUTE |
         VIRTMEM_AREA_FLAG_PAGETABLES | VIRTMEM_AREA_FLAG_GLOBAL},
        {                       /* user virtual memory: 4 MiB - 3 GiB */
         .pgindex = 1024,
         .npages = 785408,
         .flags = VIRTMEM_AREA_FLAG_USER},
        {                       /* high kernel temporary virtual memory: >3 GiB */
         .pgindex = 786432,
         .npages = 1024,
         .flags = VIRTMEM_AREA_FLAG_KERNEL |
         VIRTMEM_AREA_FLAG_PAGETABLES | VIRTMEM_AREA_FLAG_GLOBAL},
        {                       /* high kernel virtual memory: >3 GiB */
         .pgindex = 787456,
         .npages = 261120,
         .flags = VIRTMEM_AREA_FLAG_KERNEL |
         VIRTMEM_AREA_FLAG_PAGETABLES | VIRTMEM_AREA_FLAG_GLOBAL}
};

const struct vmem_area *
vmem_area_get_by_name(enum vmem_area_name name)
{
        return g_vmem_area + name;
}

const struct vmem_area *
vmem_area_get_by_page(os_index_t pgindex)
{
        const struct vmem_area *beg, *end;

        beg = g_vmem_area;
        end = g_vmem_area + sizeof(g_vmem_area) / sizeof(g_vmem_area[0]);

        while (beg < end)
        {
                if (vmem_area_contains_page(beg, pgindex))
                {
                        return beg;
                }
                ++beg;
        }

        return NULL;
}

int
vmem_area_contains_page(const struct vmem_area *area,
                           os_index_t pgindex)
{
        return (area->pgindex <= pgindex)
                && (pgindex < area->pgindex + area->npages);
}
