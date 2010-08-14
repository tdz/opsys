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

#include <string.h>
#include <sys/types.h>

#include "pageframe.h"
#include <pmem.h>

#include "page.h"
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
page_table_map_page_frame(struct page_table *pt,
                          os_index_t pfindex,
                          os_index_t index, unsigned int flags)
{
        int err;

        /*
         * ref new page frame 
         */

        if ((err = pmem_ref_frames(pfindex, 1)) < 0)
        {
                goto err_pmem_ref_frames;
        }

        /*
         * unref old page frame 
         */

        if (pte_get_pageframe_index(pt->entry[index]))
        {
                pmem_unref_frames(pte_get_pageframe_index(pt->entry[index]), 1);
        }

        /*
         * update page table entry 
         */
        pt->entry[index] = pte_create(pfindex, flags);

        return 0;

err_pmem_ref_frames:
        return err;
}

int
page_table_map_page_frames(struct page_table *pt,
                           os_index_t pfindex,
                           os_index_t index, size_t count, unsigned int flags)
{
        int err;

        for (err = 0; count && !(err < 0); --count, ++index, ++pfindex)
        {
                err = page_table_map_page_frame(pt, pfindex, index, flags);
        }

        return err;
}

int
page_table_unmap_page_frame(struct page_table *pt, os_index_t index)
{
        /*
         * unref page frame 
         */

        if (pte_get_pageframe_index(pt->entry[index]))
        {
                pmem_unref_frames(pte_get_pageframe_index(pt->entry[index]), 1);
        }

        /*
         * clear page table entry 
         */
        pt->entry[index] = pte_create(0, 0);

        return 0;
}

int
page_table_unmap_page_frames(struct page_table *pt, os_index_t index,
                             size_t count)
{
        int err;

        for (err = 0; count && !(err < 0); --count, ++index)
        {
                err = page_table_unmap_page_frame(pt, index);
        }

        return err;
}
