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
#include "page.h"
#include "pagedir.h"
#include "physmem.h"
#include "pte.h"
#include "pde.h"
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

int
page_directory_init_kernel_page_tables(struct page_directory *pd)
{
        unsigned long ptindex, ptcount, i;
        int err;
        unsigned long virt_minpt;
        unsigned long virt_pgindex, npages;

        virt_pgindex = g_virtmem_area[VIRTMEM_AREA_KERNEL].pgindex;
        npages       = g_virtmem_area[VIRTMEM_AREA_KERNEL].npages;

        ptindex = pagedir_index(page_offset(virt_pgindex));
        ptcount = pagedir_count(PAGE_MEMORY(npages));

        /* allocate and install physical pages
         */

        for (i = 0; i < ptcount; ++i) {

                struct page_table *pt;

                pt = (struct page_table *)
                        page_offset(physmem_alloc_pages(PAGE_COUNT(sizeof(*pt))));

                if (!pt) {
                        err = -5;
                        goto err_page_table_alloc;
                }

                if ((err = page_table_init(pt)) < 0) {
                        goto err_page_table_init;
                }

                pd->pentry[ptindex+i] =
                        pd_entry_create(page_index((unsigned long)pt),
                                        PDE_FLAG_PRESENT|
                                        PDE_FLAG_WRITEABLE|
                                        PDE_FLAG_CACHED);
        }

        /* set physical page-table addresses in page-table entries
         */

        virt_minpt = g_virtmem_area[VIRTMEM_AREA_KERNEL].pgindex>>10;

        for (i = 0; i < ptcount; ++virt_minpt) {

                struct page_table *pt;
                pt_entry *ptentrybeg;
                const pt_entry *ptentryend;

                /* retrieve address of first available page table */

                pt = (struct page_table *)
                        page_offset(
                                pd_entry_get_page_index(pd->pentry[virt_minpt]));

                if (!pt) {
                        err = -2;
                        goto err_pd_entry_get_address;
                }

                /* set page-table entries to physical addresses of created
                 * page tables
                 */

                ptentrybeg = pt->entry + ((virt_pgindex+i)&0x3ff);
                ptentryend = ptentrybeg + minul(ptcount-i, 1024);

                for (; ptentrybeg < ptentryend; ++ptentrybeg, ++i) {

                        unsigned long pgindex =
                                pd_entry_get_page_index(pd->pentry[ptindex+i]);

                        if (!pgindex) {
                                err = -3;
                                goto err_pd_entry_get_page_index;
                        }

                        *ptentrybeg = pt_entry_create(pgindex,
                                                      PTE_FLAG_PRESENT|
                                                      PTE_FLAG_WRITEABLE);
                }
        }

        /* install virtual pages
         */

        virt_minpt = g_virtmem_area[VIRTMEM_AREA_KERNEL].pgindex>>10;

        for (i = 0; i < ptcount; ++i) {

                struct page_table *pt;
                unsigned long j;

                /* retrieve page-directory entry */

                pt = (struct page_table*)
                        page_offset(
                                pd_entry_get_page_index(pd->pentry[virt_minpt]));

                if (!pt) {
                        err = -4;
                        goto err_pd_entry_get_ptoffset;
                }

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

err_pd_entry_get_page_index:
err_pd_entry_get_ptoffset:
err_pd_entry_get_address:
err_page_table_init:
err_page_table_alloc:
        return err;
}

void
page_directory_uninit(struct page_directory *pd)
{
        return;
}

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

                while ((pebeg < peend) && !pt_entry_get_page_index(*pebeg)) {
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
__page_directory_install_page_table(struct page_directory *pd,
                                    unsigned long pdindex,
                                    unsigned long flags)
{
        struct page_table *pt;
        int err;
        unsigned long virt_pgindex;

        pt = (struct page_table *)
                page_offset(physmem_alloc_pages(PAGE_COUNT(sizeof(*pt))));

        if (!pt) {
                err = -1;
                goto err_page_table_alloc;
        }

        /* create new page table */
        if ((err = page_table_init(pt)) < 0) {
                goto err_page_table_init;
        }

        /* and install in page directory */

        virt_pgindex = err =
                page_directory_install_physical_pages_in_area(pd,
                        VIRTMEM_AREA_KERNEL,
                        page_index((unsigned long)pt),
                        PAGE_COUNT(sizeof(*pt)),
                        flags);

        if (err < 0) {
                goto err_page_directory_install_physical_pages_in_area;
        }

        /* store physical page-table address */
        pd->pentry[pdindex] =
                pd_entry_create(page_offset((unsigned long)pt),
                                PDE_FLAG_PRESENT|
                                PDE_FLAG_WRITEABLE|
                                PDE_FLAG_CACHED);

        /* store virtual page-table address */
        pd->ventry[pdindex] = page_offset(virt_pgindex);

        return virt_pgindex;

err_page_directory_install_physical_pages_in_area:
err_page_table_init:
err_page_table_alloc:
        return err;
}

static int
__page_directory_install_physical_page_at(struct page_directory *pd,
                                          unsigned long virt_pgindex,
                                          unsigned long phys_pgindex,
                                          unsigned long flags)
{
        unsigned long virt_pdindex;
        int err;
        struct page_table *pt;

        virt_pdindex = pagedir_index(page_offset(virt_pgindex));

        /* create new page table, if not yet present */

        if (!pd->ventry[virt_pdindex]) {
                err = __page_directory_install_page_table(pd,
                                                          virt_pdindex,
                                                          PTE_FLAG_PRESENT|
                                                          PTE_FLAG_WRITEABLE);
                if (err < 0) {
                        goto err___page_directory_install_page_table;
                }
        }

        /* set page-table entry */

        if (!(pt = (struct page_table*)pd->ventry[virt_pdindex])) {
                err = -1;
                goto err_pt;
        }

        pt->entry[virt_pgindex&0x3ff] = pt_entry_create(phys_pgindex, flags);

        return virt_pgindex;

err___page_directory_install_page_table:
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
                if (err < 0) {
                        goto err___page_directory_install_physical_page_at;
                }
        }

        return virt_pgindex;

err___page_directory_install_physical_page_at:
        return err;        
}

int
page_directory_install_physical_pages_in_area(struct page_directory *pd,
                                              enum virtmem_area_name area,
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
                                        g_virtmem_area[area].pgindex,
                                        g_virtmem_area[area].pgindex +
                                        g_virtmem_area[area].npages,
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
        if (err < 0) {
                goto err_page_directory_install_physical_page_at;
        }

        return virt_pgindex;

err___page_directory_find_empty_pages:
err_page_directory_install_physical_page_at:
        return err;
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

