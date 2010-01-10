/*
 *  oskernel - A small experimental operating-system kernel
 *  Copyright (C) 2009  Thomas Zimmermann <tdz@users.sourceforge.net>
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
#include "pte.h"
#include "pde.h"
#include "page.h"
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
page_directory_init(struct page_directory *pd)
{
        memset(pd, 0, sizeof(*pd));

        return 0;
}

void
page_directory_uninit(struct page_directory *pd)
{
        return;
}

static int
__page_directory_alloc_page_table_at(struct page_directory *pd,
                                     unsigned long ptindex,
                                     unsigned int flags)
{
        unsigned long pfindex;
        int err;

        if (pd_entry_get_pageframe_index(pd->pentry[ptindex])) {
                return 0; /* page table already exists */
        }

        pfindex = physmem_alloc_frames(pageframe_count(sizeof(struct page_table)));

        if (!pfindex) {
                err = -1;
                goto err_physmem_alloc_frames;
        }

        if ((err = page_table_init(pageframe_address(pfindex))) < 0) {
                goto err_page_table_init;
        }

        pd->pentry[ptindex] = pd_entry_create(pfindex, flags);

        return 0;

err_page_table_init:
        physmem_unref_frames(pfindex, pageframe_count(sizeof(struct page_table)));
err_physmem_alloc_frames:
        return err;
}

static int
__page_directory_alloc_page_tables_at(struct page_directory *pd,
                                      unsigned long ptindex,
                                      unsigned long ptcount,
                                      unsigned int flags)
{
        int err = 0;

        while (ptcount && !(err < 0)) {
                err = __page_directory_alloc_page_table_at(pd, ptindex, flags);
                ++ptindex;
                --ptcount;
        }

        return err;
}

static int
__page_directory_map_pageframe_at(struct page_directory *pd,
                                  unsigned long pfindex,
                                  unsigned long pgindex,
                                  unsigned int flags)
{
        unsigned long ptindex;
        int err;
        struct page_table *pt;

        /* get page table */

        ptindex = pagedir_index(page_offset(pgindex));

        pt = pageframe_address(pd_entry_get_pageframe_index(pd->pentry[ptindex]));

        if (!pt) {
                /* no page table present */
                err = -2;
                goto err_nopagetable;
        }

        /* ref new page frame */

        if ((err = physmem_ref_frames(pfindex, 1)) < 0) {
                goto err_physmem_ref_frames;
        }

        /* unref old page frame */

        if (pt_entry_get_pageframe_index(pt->entry[pgindex&0x3ff])) {
                physmem_unref_frames(
                        pt_entry_get_pageframe_index(
                                pt->entry[pgindex&0x3ff]), 1);
        }

        /* update page table entry */
        pt->entry[pgindex&0x3ff] = pt_entry_create(pfindex, flags);

        return 0;

err_physmem_ref_frames:
err_nopagetable:
        return err;
}

static int
__page_directory_map_pageframes_at(struct page_directory *pd,
                                   unsigned long pfindex,
                                   unsigned long pgindex,
                                   unsigned long count,
                                   unsigned int flags)
{
        int err = 0;

        while (count && !(err < 0)) {
                err = __page_directory_map_pageframe_at(pd,
                                                        pfindex,
                                                        pgindex,
                                                        flags);
                ++pgindex;
                ++pfindex;
                --count;
        }

        return err;
}

static int 
__page_directory_install_page_tables_at(struct page_directory *pd,
                                        unsigned long pgindex_tgt,
                                        unsigned long ptindex,
                                        unsigned long ptcount,
                                        unsigned int flags)
{
        int err;

        while (ptcount) {

                unsigned long ptindex_tgt;
                struct page_table *pt;
                unsigned long j;

                /* retrieve address of target page table in kernel area */

                ptindex_tgt = pagedir_index(page_offset(pgindex_tgt));

                pt = pageframe_address(pd_entry_get_pageframe_index(pd->pentry[ptindex_tgt]));

                if (!pt) {
                        err = -2;
                        goto err_pd_entry_get_address;
                }

                /* set page-table entries to page frames of
                   created page tables */

                for (j = pgindex_tgt&0x3ff; (j < 1024) && ptcount; ++j) {

                        unsigned long pfindex;

                        if (pt_entry_get_pageframe_index(pt->entry[j])) {
                                continue; /* entry not empty, try next one */
                        }

                        pfindex = pd_entry_get_pageframe_index(pd->pentry[ptindex]);

                        if (!pfindex) {
                                err = -3;
                                goto err_pd_entry_get_pageframe_index;
                        }

                        pt->entry[j] = pt_entry_create(pfindex, flags);

                        --ptcount;
                        ++ptindex;

                        /* TODO: setup virtual addresses here */
                }

                pgindex_tgt += j;
        }

        return 0;

err_pd_entry_get_pageframe_index:
err_pd_entry_get_address:
        return err;
}

#include "console.h"
int
page_directory_install_kernel_area_low(struct page_directory *pd)
{
        unsigned long ptindex, ptcount;
        unsigned long pgindex, pgcount;
        int err;

        pgindex = g_virtmem_area[VIRTMEM_AREA_LOW].pgindex;
        pgcount = g_virtmem_area[VIRTMEM_AREA_LOW].npages;

        ptindex = pagedir_index(page_offset(pgindex));
        ptcount = pagedir_count(page_memory(pgcount));

        /* create page tables for low area */

        err = __page_directory_alloc_page_tables_at(pd,
                                                    ptindex,
                                                    ptcount,
                                                    PDE_FLAG_PRESENT|
                                                    PDE_FLAG_WRITEABLE);
        if (err < 0) {
                goto err___page_directory_alloc_page_tables_at;
        }

        /* create identity mapping for low area */

        err = __page_directory_map_pageframes_at(pd,
                                                 pgindex,
                                                 pgindex,
                                                 pgcount,
                                                 PTE_FLAG_PRESENT|
                                                 PTE_FLAG_WRITEABLE);
        if (err < 0) {
                goto err___page_directory_map_pageframes_at;
        }

        return 0;

err___page_directory_alloc_page_tables_at:
err___page_directory_map_pageframes_at:
        return err;
}

static unsigned long
__page_directory_check_empty_pages_at(const struct page_directory *pd,
                                      unsigned long pgindex,
                                      unsigned long npages)
{
        unsigned long nempty;
        const unsigned long *ventrybeg, *ventryend;

        nempty = 0;

        ventrybeg = pd->ventry+pagedir_index(page_offset(pgindex));
        ventryend = pd->ventry+pagedir_index(page_offset(pgindex+npages));

        while (ventrybeg < ventryend) {

                const struct page_table *pt;
                const unsigned long *pebeg, *peend;

                pt = (const struct page_table*)(*ventrybeg);

                if (!pt) {
                        nempty += 1024-(pgindex&0x3ff);
                }

                /* count empty pages */

                pebeg = pt->entry+(pgindex&0x3ff);
                peend = pebeg + minul(npages-nempty, 1024-(pgindex&0x3ff));

                while ((pebeg < peend) && !pt_entry_get_pageframe_index(*pebeg)) {
                        ++nempty;
                        ++pgindex;
                        ++pebeg;
                }

                /* break if stopped early */
                if (pebeg < peend) {
                        break;
                }

                ++ventrybeg;
        }

        return nempty;
}

static unsigned long
__page_directory_find_empty_pages(const struct page_directory *pd,
                                  unsigned long virt_beg_pgindex,
                                  unsigned long virt_end_pgindex,
                                  unsigned long npages)
{
        /* FIXME: this function subject to integer overflow errors
         */

        /* find continuous area in virtual memory */

        while ((virt_beg_pgindex < virt_end_pgindex) &&
               (npages <= virt_end_pgindex-virt_beg_pgindex)) {

                unsigned long nempty;

                nempty = __page_directory_check_empty_pages_at(pd,
                                                        virt_beg_pgindex,
                                                        npages);
                if (nempty == npages) {
                        return virt_beg_pgindex;
                }

                /* goto page after non-empty one */
                virt_beg_pgindex += nempty+1;
        }

        return 0;
}

unsigned long
page_directory_lookup_physical_page(const struct page_directory *pd,
                                    unsigned long virt_pgindex)
{
        const struct page_table *pt;

        pt = (const struct page_table*)
                pd->ventry[pagedir_index(page_offset(virt_pgindex))];

        if (!pt) {
                return 0;
        }

        return (pt->entry[virt_pgindex&0x3ff]) >> 12;
}

