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
#include "minmax.h"
#include "pageframe.h"
#include "physmem.h"
#include "page.h"
#include "pte.h"
#include "pagetbl.h"
#include "pde.h"
#include "pagedir.h"

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

int
page_directory_alloc_page_table_at(struct page_directory *pd,
                                   unsigned long ptindex,
                                   unsigned int flags)
{
        unsigned long pfindex;
        int err;

        if (pde_get_pageframe_index(pd->pentry[ptindex])) {
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

        pd->pentry[ptindex] = pde_create(pfindex, flags);

        return 0;

err_page_table_init:
        physmem_unref_frames(pfindex, pageframe_count(sizeof(struct page_table)));
err_physmem_alloc_frames:
        return err;
}

int
page_directory_alloc_page_tables_at(struct page_directory *pd,
                                    unsigned long ptindex,
                                    unsigned long ptcount,
                                    unsigned int flags)
{
        int err = 0;

        while (ptcount && !(err < 0)) {
                err = page_directory_alloc_page_table_at(pd, ptindex, flags);
                ++ptindex;
                --ptcount;
        }

        return err;
}

int
page_directory_map_pageframe_at(struct page_directory *pd,
                                unsigned long pfindex,
                                unsigned long pgindex,
                                unsigned int flags)
{
        unsigned long ptindex;
        int err;
        struct page_table *pt;

        /* get page table */

        ptindex = pagetable_index(page_offset(pgindex));

        pt = pageframe_address(pde_get_pageframe_index(pd->pentry[ptindex]));

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

        if (pte_get_pageframe_index(pt->entry[pgindex&0x3ff])) {
                physmem_unref_frames(
                        pte_get_pageframe_index(
                                pt->entry[pgindex&0x3ff]), 1);
        }

        /* update page table entry */
        pt->entry[pgindex&0x3ff] = pte_create(pfindex, flags);

        return 0;

err_physmem_ref_frames:
err_nopagetable:
        return err;
}

int
page_directory_map_pageframes_at(struct page_directory *pd,
                                 unsigned long pfindex,
                                 unsigned long pgindex,
                                 unsigned long count,
                                 unsigned int flags)
{
        int err = 0;

        while (count && !(err < 0)) {
                err = page_directory_map_pageframe_at(pd,
                                                      pfindex,
                                                      pgindex,
                                                      flags);
                ++pgindex;
                ++pfindex;
                --count;
        }

        return err;
}

int 
page_directory_install_page_tables_at(struct page_directory *pd,
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

                ptindex_tgt = pagetable_index(page_offset(pgindex_tgt));

                pt = pageframe_address(pde_get_pageframe_index(pd->pentry[ptindex_tgt]));

                if (!pt) {
                        err = -2;
                        goto err_pd_entry_get_address;
                }

                /* set page-table entries to page frames of
                   created page tables */

                for (j = pgindex_tgt&0x3ff; (j < 1024) && ptcount; ++j) {

                        unsigned long pfindex;

                        if (pte_get_pageframe_index(pt->entry[j])) {
                                continue; /* entry not empty, try next one */
                        }

                        pfindex = pde_get_pageframe_index(pd->pentry[ptindex]);

                        if (!pfindex) {
                                err = -3;
                                goto err_pd_entry_get_pageframe_index;
                        }

                        pt->entry[j] = pte_create(pfindex, flags);

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

unsigned long
page_directory_check_empty_pages_at(const struct page_directory *pd,
                                    unsigned long pgindex,
                                    unsigned long npages)
{
        unsigned long nempty;
        const unsigned long *ventrybeg, *ventryend;

        nempty = 0;

        ventrybeg = pd->ventry + pagetable_index(page_offset(pgindex));
        ventryend = pd->ventry + pagetable_index(page_offset(pgindex+npages));

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

                while ((pebeg < peend) && !pte_get_pageframe_index(*pebeg)) {
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

unsigned long
page_directory_find_empty_pages(const struct page_directory *pd,
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

                nempty = page_directory_check_empty_pages_at(pd,
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

