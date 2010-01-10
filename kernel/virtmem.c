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

#include "stddef.h"
#include "types.h"
#include "minmax.h"
#include "string.h"
#include "pageframe.h"
#include "physmem.h"
#include "page.h"
#include "pte.h"
#include "pagetbl.h"
#include "pde.h"
#include "pagedir.h"
#include "virtmem.h"
#include "tcb.h"
#include "task.h"

#define MAXTASK 1024

const struct virtmem_area g_virtmem_area[4] = {
        /* low kernel virtual memory: <4 MiB */
        {.pgindex = 1,                                              .npages = 1023},
        /* user virtual memory: 4 MiB - 3 GiB */
        {.pgindex = 1024,                                           .npages = 785408},
        /* task state memory: >3 GiB */
        {.pgindex = 786432,                                         .npages = PAGE_COUNT(MAXTASK*sizeof(struct task))},
        /* high kernel virtual memory: > 3 GiB */
        {.pgindex = 786432+PAGE_COUNT(MAXTASK*sizeof(struct task)), .npages = 262144-PAGE_COUNT(MAXTASK*sizeof(struct task))}
};

int
virtmem_install_kernel_area_low(struct page_directory *pd)
{
        unsigned long ptindex, ptcount;
        unsigned long pgindex, pgcount;
        int err;

        pgindex = g_virtmem_area[VIRTMEM_AREA_LOW].pgindex;
        pgcount = g_virtmem_area[VIRTMEM_AREA_LOW].npages;

        ptindex = pagetable_index(page_offset(pgindex));
        ptcount = pagetable_count(page_memory(pgcount));

        /* create page tables for low area */

        err = page_directory_alloc_page_tables_at(pd,
                                                  ptindex,
                                                  ptcount,
                                                  PDE_FLAG_PRESENT|
                                                  PDE_FLAG_WRITEABLE);
        if (err < 0) {
                goto err_page_directory_alloc_page_tables_at;
        }

        /* create identity mapping for low area */

        err = page_directory_map_pageframes_at(pd,
                                               pgindex,
                                               pgindex,
                                               pgcount,
                                               PTE_FLAG_PRESENT|
                                               PTE_FLAG_WRITEABLE);
        if (err < 0) {
                goto err_page_directory_map_pageframes_at;
        }

        return 0;

err_page_directory_alloc_page_tables_at:
err_page_directory_map_pageframes_at:
        return err;
}

unsigned long
virtmem_lookup_physical_page(const struct page_directory *pd,
                                    unsigned long virt_pgindex)
{
        const struct page_table *pt;

        pt = (const struct page_table*)
                pd->ventry[pagetable_index(page_offset(virt_pgindex))];

        if (!pt) {
                return 0;
        }

        return (pt->entry[virt_pgindex&0x3ff]) >> 12;
}

