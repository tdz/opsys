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

const struct virtmem_area g_virtmem_area[] = {
        /* low kernel virtual memory: <4 MiB */
        {.pgindex = 1,
         .npages  = 1023,
         .flags   = VIRTMEM_AREA_FLAG_KERNEL|VIRTMEM_AREA_FLAG_IDENTITY},
        /* user virtual memory: 4 MiB - 3 GiB */
        {.pgindex = 1024,
         .npages  = 785408,
         .flags   = VIRTMEM_AREA_FLAG_USER},
        /* task state memory: >3 GiB */
        {.pgindex = 786432,
         .npages  = PAGE_COUNT(MAXTASK*sizeof(struct task)),
         .flags   = VIRTMEM_AREA_FLAG_KERNEL},
        /* high kernel virtual memory: >3 GiB */
        {.pgindex = 786432+PAGE_COUNT(MAXTASK*sizeof(struct task)),
         .npages  = 262144-PAGE_COUNT(MAXTASK*sizeof(struct task)),
         .flags   = VIRTMEM_AREA_FLAG_KERNEL}
};

int
virtmem_install_kernel_area_low(struct page_directory *pd)
{
        int err;
        const struct virtmem_area *beg, *end;

        beg = g_virtmem_area;
        end = beg + sizeof(g_virtmem_area)/sizeof(g_virtmem_area[0]);

        /* install page tables in all kernel areas */

        for (err = 0; (beg < end) && !(err < 0); ++beg) {

                unsigned long ptindex, ptcount;

                if (!(beg->flags&VIRTMEM_AREA_FLAG_KERNEL)) {
                        continue;
                }

                ptindex = pagetable_index(page_offset(beg->pgindex));
                ptcount = pagetable_count(page_memory(beg->npages));

                /* create page tables for low area */

                err = page_directory_alloc_page_tables_at(pd,
                                                          ptindex,
                                                          ptcount,
                                                          PDE_FLAG_PRESENT|
                                                          PDE_FLAG_WRITEABLE);
        }

        /* create identity mapping for all identity areas */

        beg = g_virtmem_area;
        end = beg + sizeof(g_virtmem_area)/sizeof(g_virtmem_area[0]);

        for (; (beg < end) && !(err < 0); ++beg) {

                if (!(beg->flags&VIRTMEM_AREA_FLAG_IDENTITY)) {
                        continue;
                }

                err = page_directory_map_pageframes_at(pd,
                                                       beg->pgindex,
                                                       beg->pgindex,
                                                       beg->npages,
                                                       PTE_FLAG_PRESENT|
                                                       PTE_FLAG_WRITEABLE);
        }

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

