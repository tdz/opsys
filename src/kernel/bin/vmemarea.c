/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009-2010  Thomas Zimmermann
 *  Copyright (C) 2016       Thomas Zimmermann
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

#include "vmemarea.h"
#include <stddef.h>
#include <string.h>
#include "vmemarea_32.h"

const struct vmem_area*
vmem_area_get_by_name(enum vmem_area_name name)
{
    return g_vmem_area + name;
}

const struct vmem_area*
vmem_area_get_by_page(os_index_t pgindex)
{
    const struct vmem_area* beg = g_vmem_area;
    const struct vmem_area* end = g_vmem_area + ARRAY_NELEMS(g_vmem_area);

    while (beg < end) {
        if (vmem_area_contains_page(beg, pgindex)) {
            return beg;
        }
        ++beg;
    }

    return NULL;
}

int
vmem_area_contains_page(const struct vmem_area* area, os_index_t pgindex)
{
    return (area->pgindex <= pgindex)
            && (pgindex < area->pgindex + area->npages);
}
