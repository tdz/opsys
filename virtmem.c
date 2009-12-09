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
#include "pagedir.h"
#include "physmem.h"
#include "pte.h"
#include "pde.h"
#include "virtmem.h"
#include "tcb.h"
#include "task.h"

const unsigned long g_min_kernel_virtaddr = 0xc0000000;
const unsigned long g_max_kernel_virtaddr = 0xffffffff;
const unsigned long g_max_user_virtaddr   = 0xc0000000-1;

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

/*        unsigned long pgindex;
        struct page_directory *pd;

        pgindex = physmem_alloc(PAGE_COUNT(sizeof(*pd)));

        if (!pgindex) {
                return NULL;
        }

        pd = (struct page_directory*)page_offset(pgindex);

        return memset(pd, 0, sizeof(*pd));*/
}

void
page_directory_uninit(struct page_directory *pd)
{
        return;
/*        physmem_unref(page_index((unsigned long)pd), 1);*/
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

                pt = (struct page_table *)
                        page_offset(physmem_alloc_pages(PAGE_COUNT(sizeof(*pt))));

                if (!pt) {
                        err = -1;
                        goto err_page_table_alloc;
                }

                if ( (err = page_table_init(pt)) < 0) {
                        goto err_page_table_init;
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
err_page_table_init:
err_page_table_alloc:
        return err;
}

static const unsigned long g_task_state_npages =
        PAGE_COUNT(MAXTASK*sizeof(struct task));

static const unsigned long g_virtmem_area[4][2] =
        {{1,    1023},      /* < 4 MiB */
         {1024, 785408},    /* 4 MiB - 3 GiB */
         {786432, PAGE_COUNT(MAXTASK*sizeof(struct task))}, /* > 3 GiB */
         {786432+PAGE_COUNT(MAXTASK*sizeof(struct task)), 262144-PAGE_COUNT(MAXTASK*sizeof(struct task))}}; /* > 3 GiB */

#include "minmax.h"

static unsigned long
__page_directory_check_empty_pages_at(const struct page_directory *pd,
                                      unsigned long virt_pgindex,
                                      unsigned long npages)
{
        unsigned long nempty;
        const unsigned long *ventrybeg, *ventryend;

        nempty = 0;

        ventrybeg = pd->ventry+pagedir_index(page_offset(virt_pgindex));
        ventryend = pd->ventry+pagedir_index(page_offset(virt_pgindex+npages));

        while (ventrybeg < ventryend) {

                const struct page_table *pt;
                const unsigned long *pebeg, *peend;

                pt = (const struct page_table*)(*ventrybeg);

                if (!pt) {
                        nempty += 1024-(virt_pgindex&0x3ff);
                }

                /* count empty pages */

                pebeg = pt->entry+(virt_pgindex&0x3ff);
                peend = pebeg + minul(npages-nempty, 1024-(virt_pgindex&0x3ff));

                while ((pebeg < peend) && !pt_entry_get_address(*pebeg)) {
                        ++nempty;
                        ++virt_pgindex;
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

static int
__page_directory_install_physical_page_at(struct page_directory *pd,
                                          unsigned long virt_pgindex,
                                          unsigned long phys_pgindex,
                                          unsigned long flags)
{
        unsigned long virt_pgoffset, virt_pdindex;
        struct page_table *pt;
        int err;

        virt_pgoffset = page_offset(virt_pgindex);

        virt_pdindex = pagedir_index(virt_pgoffset);

        /* create new page table, if not yet present */

        if (!pd->ventry[virt_pdindex]) {
                unsigned long ventry_pgindex;

                pt = (struct page_table *)
                        page_offset(physmem_alloc_pages(PAGE_COUNT(sizeof(*pt))));

                if (!pt) {
                        err = -1;
                        goto err_page_table_alloc;
                }

                /* create new page table */
                if ( (err = page_table_init(pt)) < 0) {
                        goto err_page_table_init;
                }

                /* and install in page directory */

                ventry_pgindex =
                        __page_directory_find_empty_pages(
                                pd, g_virtmem_area[VIRTMEM_AREA_KERNEL][0],
                                    g_virtmem_area[VIRTMEM_AREA_KERNEL][0] +
                                    g_virtmem_area[VIRTMEM_AREA_KERNEL][1],
                                    PAGE_COUNT(sizeof(*pt)) );

                if (!ventry_pgindex) {
                        goto err___page_directory_find_empty_page;
                }

                err = __page_directory_install_physical_page_at(pd,
                                ventry_pgindex,
                                page_index((unsigned long)pt),
                                PDE_FLAG_PRESENT|
                                PDE_FLAG_WRITEABLE|
                                PDE_FLAG_CACHED);

                if (err) {
                        goto err___page_directory_install_physical_page_at;
                }

                pd->ventry[virt_pdindex] = ventry_pgindex;
        }

        /* set page-table entry */

        pt = (struct page_table*)pd->ventry[virt_pdindex];

        if (!pt) {
                err = -1;
                goto err_pt;
        }

        pt->entry[virt_pgindex&PAGE_MASK] =
                pt_entry_create(page_offset(phys_pgindex), flags);

        return 0;

err_page_table_init:
err_page_table_alloc:
err___page_directory_install_physical_page_at:
err___page_directory_find_empty_page:
err_pt:
        return err;
}

int
page_directory_install_physical_pages_at(struct page_directory *pd,
                                         unsigned long virt_pgindex,
                                         unsigned long phys_pgindex,
                                         unsigned long npages,
                                         unsigned long flags)
{
        int err;
        unsigned long i;

        for (i = 0; i < npages; ++i) {
                err = __page_directory_install_physical_page_at(
                                pd,
                                virt_pgindex+i,
                                phys_pgindex+i,
                                flags);
                if (err) {
                        goto err___page_directory_install_physical_page_at;
                }
        }

        return 0;

err___page_directory_install_physical_page_at:
        return err;        
}

int
page_directory_install_physical_pages_in_area(struct page_directory *pd,
                                              enum virtmem_area area,
                                              unsigned long phys_pgindex,
                                              unsigned long npages,
                                              unsigned long flags)
{
        unsigned long virt_pgindex;
        int err;

        if (!(area < sizeof(g_virtmem_area)/sizeof(g_virtmem_area[0]))) {
                return 0;
        }

        virt_pgindex = __page_directory_find_empty_pages(pd,
                                        g_virtmem_area[area][0],
                                        g_virtmem_area[area][0] +
                                        g_virtmem_area[area][1],
                                        npages);

        if (!virt_pgindex) {
                err = -1;
                goto err___page_directory_find_empty_pages;
        }

        err = page_directory_install_physical_pages_at(pd,
                                                       virt_pgindex,
                                                       phys_pgindex,
                                                       npages,
                                                       flags);
        if (err) {
                goto err_page_directory_install_physical_page_at;
        }

        return 0;

err___page_directory_find_empty_pages:
err_page_directory_install_physical_page_at:
        return err;
}

