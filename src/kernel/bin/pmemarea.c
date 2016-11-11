/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2010  Thomas Zimmermann
 *  Copyright (C) 2016  Thomas Zimmermann
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

#include "pmemarea.h"
#include <stddef.h>
#include <string.h>

/* INDENT-OFF */

static const struct pmem_area g_pmem_area[LAST_PMEM_AREA] =
{
        [PMEM_AREA_DMA_20] = { .pfindex = 0,    .nframes = 256},
        [PMEM_AREA_DMA_24] = { .pfindex = 0,    .nframes = 4096},
        [PMEM_AREA_DMA_32] = { .pfindex = 0,    .nframes = 785408},
        [PMEM_AREA_USER]   = { .pfindex = 1024, .nframes = 1048576}
};

/* INDENT-ON */

const struct pmem_area *
pmem_area_get_by_name(enum pmem_area_name name)
{
        return g_pmem_area + name;
}

const struct pmem_area *
pmem_area_get_by_frame(os_index_t pgindex)
{
        const struct pmem_area *beg, *end;

        beg = g_pmem_area;
        end = g_pmem_area + ARRAY_NELEMS(g_pmem_area);

        while (beg < end)
        {
                if (pmem_area_contains_frame(beg, pgindex))
                {
                        return beg;
                }
                ++beg;
        }

        return NULL;
}

int
pmem_area_contains_frame(const struct pmem_area *area, os_index_t pfindex)
{
        return (area->pfindex <= pfindex)
                && (pfindex < area->pfindex + area->nframes);
}
