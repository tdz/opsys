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
#include "string.h"
#include "page.h"
#include "physmem.h"
#include "pte.h"
#include "pde.h"
#include "virtmem.h"

const unsigned long g_min_kernel_virtaddr = 0xc0000000;
const unsigned long g_max_kernel_virtaddr = 0xffffffff;
const unsigned long g_max_user_virtaddr   = 0xc0000000-1;

struct page_table *
page_table_create()
{
        struct page_directory *pt;

        pt = (struct page_directory *)page_offset(physmem_alloc(1));

        if (!pt) {
                return NULL;
        }

        return memset(pt, 0, sizeof(*pt));
}

void
page_table_destroy(struct page_table *pt)
{
        physmem_unref(page_index((unsigned long)pt), 1);
}


struct page_directory *
page_directory_create()
{
        unsigned long pgindex;
        struct page_directory *pd;

        pgindex = physmem_alloc(page_count(sizeof(*pd)));

        if (!pgindex) {
                return NULL;
        }

        pd = (struct page_directory*)page_offset(pgindex);

        return memset(pd, 0, sizeof(*pd));
}

void
page_directory_destroy(struct page_directory *pd)
{
        physmem_unref(page_index((unsigned long)pd), 1);
}

static unsigned long
__find_continuous_virt_pages(struct page_directory *pd, unsigned long npages)
{
        size_t i;
        unsigned long virt_pgindex;
        unsigned long pgfound;

        /* find continuous area in virtual memory */

        for (i = 0; i < sizeof(pd->ventry)/sizeof(pd->ventry[0]);) {
                pgfound = 0;

                for (; i < sizeof(pd->ventry)/sizeof(pd->ventry[0]); ++i) {

                        const struct page_table *pt;
                        size_t j;

                        /* 1024 empty pages if no page table at this entry
                         */

                        if (!pd->ventry[i]) {
                                if (!pgfound) {
                                        virt_pgindex = i*1024;
                                }
                                pgfound += 1024;

                                /* return if found enough pages */
                                if (pgfound >= npages) {
                                        goto do_return;
                                }

                                continue;
                        }

                        /* recure to page table
                         */

                        pt = (const struct page_table*)(pd->ventry+i);

                        for (j = 0;
                             j < sizeof(pt->entry)/sizeof(pt->entry[0]);
                           ++j) {

                                /* page not empty, reset and try next page */
                                if (pt->entry[j]) {
                                        pgfound = 0;
                                        continue;
                                }

                                /* begin new area */
                                if ( !(pgfound++) ) {
                                        virt_pgindex = i*1024+j;
                                }

                                /* return if found enough pages */
                                if (pgfound == npages) {
                                        goto do_return;
                                }
                        }
                }
        }

do_return:
        return virt_pgindex;

}

static int
__install_page_directory_entry(struct page_directory *pd, unsigned long i,
                                                          unsigned long flags)
{
/*        unsigned long phys_pgindex;*/

        if (pd->ventry[i]) {
                return -1; /* entry already exists */
        }

        return 0;
}

unsigned long
page_directory_alloc_pages(struct page_directory *pd, unsigned long npages)
{
        unsigned long virt_pgindex;

        /* FIXME: lock here */

        /* find continuous area in virtual memory */
        virt_pgindex = __find_continuous_virt_pages(pd, npages);

        if (!virt_pgindex) {
                goto err_find_continuous_virt_pages;
        }

err_find_continuous_virt_pages:
        /* FIXME: unlock here */

        return virt_pgindex;
}

unsigned long
page_directory_alloc_phys_pages(struct page_directory *pd,
                                unsigned long phys_pgindex,
                                unsigned long npages)
{
        return 0;
}

unsigned long
page_directory_alloc_phys_pages_at(struct page_directory *pd,
                                   unsigned long virt_pgindex,
                                   unsigned long phys_pgindex,
                                   unsigned long npages)
{
        return 0;
}

void
page_directory_release_pages(struct page_directory *pd, unsigned long pgindex,
                                                        unsigned npages)
{
        return;
}

unsigned long
page_directory_lookup_phys(const struct page_directory *pd,
                           unsigned long virtaddr)
{
        const struct page_table *pt;

        pt = (const struct page_table*)(pd->ventry + (virtaddr>>22));

        if (!pt) {
                return 0;
        }

        return (pt->entry[(virtaddr>>12)&0x3ff]&0xfffffc00) + (virtaddr&0x3ff);
}

#include "pte.h"
#include "pde.h"
#include "tcb.h"
#include "task.h"

int
page_directory_install_page_tables(struct page_directory *pd,
                                   unsigned long virt_pgindex,
                                   unsigned long npages)
{
        /* FIXME: todo */

        unsigned long ptindex, ptcount, i;
        int err;
        unsigned long virt_minpg;
        unsigned long virt_maxpg;
        unsigned long virt_minpt;

        ptindex = virt_pgindex&0x3ff;
        ptcount = (npages+1023) >> 10;

        /* install physical pages */

        for (i = 0; i < ptcount; ++i) {

                struct page_table *pt;

                if ( !(pt = page_table_create()) ) {
                        err = -1;
                        goto err_page_table_create;
                }

                pd->pentry[ptindex+i] = pd_entry_create((unsigned long)pt,
                                                        PDE_FLAG_PRESENT|
                                                        PDE_FLAG_WRITEABLE|
                                                        PDE_FLAG_CACHED);
        }

        /* set page-tables addresses in page-table entries */

        #define MAXTASK 1024

        virt_minpg = page_index(g_min_kernel_virtaddr+MAXTASK*sizeof(struct task));
        virt_maxpg = page_index(g_max_kernel_virtaddr);
        virt_minpt = virt_minpg>>10;

        for (i = 0; i < ptcount; ++i) {

                unsigned long ptoffset;
                struct page_table *pt;
                unsigned long j;

                /* retrieve first available page table */

                ptoffset = pd_entry_get_address(pd->pentry[virt_minpt]);

                if (!ptoffset) {
                        err = -1;
                        goto err_pd_entry_get_ptoffset;
                }

                pt = (struct page_table *)ptoffset;

                /* polute page-table entries with physical
                   addresses of created page tables
                 */

                for (j = (ptindex+i)&0x3ff; (j < 1024) && (i < ptcount);
                   ++j, ++i) {

                        unsigned long pgoffset =
                                pd_entry_get_address(pd->pentry[ptindex+i]);

                        if (!pgoffset) {
                                err = -1;
                                goto err_pd_entry_get_pgoffset;
                        }

                        pt->entry[j] = pt_entry_create(pgoffset,
                                                       PTE_FLAG_PRESENT|
                                                       PTE_FLAG_WRITEABLE);
                }

                ++virt_minpt;
        }

        /* install virtual pages */

        virt_minpt = virt_minpg>>10;

        for (i = 0; i < ptcount; ++i) {

                unsigned long ptoffset;
                struct page_table *pt;
                unsigned long j;

                /* retrieve page-directory entry */

                ptoffset = pd_entry_get_address(pd->pentry[virt_minpt]);

                if (!ptoffset) {
                        err = -1;
                        goto err_pd_entry_get_ptoffset;
                }

                pt = (struct page_table*)ptoffset;

                /* polute page-directory entries with virtual
                   addresses of created page tables
                 */

                for (j = (ptindex+i)&0x3ff; (j < 1024) && (i < ptcount);
                   ++j, ++i) {

                        pd->ventry[ptindex+i] = ((ptindex<<10) + j) << 12;
                }

                ++virt_minpt;
        }

        return 0;

        /* FIXME: write error handling */

err_pd_entry_get_pgoffset:
err_pd_entry_get_ptoffset:
err_page_table_create:
        return err;
}
