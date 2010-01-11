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

#include "types.h"
#include "string.h"
#include "pageframe.h"
#include "physmem.h"
#include "pte.h"
#include "pagetbl.h"

int
page_table_init(struct page_table *pt)
{
        memset(pt, 0, sizeof(*pt));

        return 0;
}

void
page_table_uninit(struct page_table *pt)
{
        return;
}

int
page_table_alloc_pages_at(struct page_table *pt, unsigned long pgindex,
                                                 unsigned long pgcount,
                                                 unsigned int flags)
{
        while (pgcount) {
                unsigned long pfindex = physmem_alloc_frames(1);

                pt->entry[pgindex] = pte_create(pfindex, flags);

                ++pgindex;
                --pgcount;
        }

        return 0;
}

