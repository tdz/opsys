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
#include "mmu.h"
#include "virtmem.h"
#include "tcb.h"
#include "task.h"

#define MAXTASK 1024

const struct virtmem_area g_virtmem_area[LAST_VIRTMEM_AREA] = {
        {/* low kernel virtual memory: <4 MiB */
         .pgindex = 1,
         .npages  = 1023,
         .flags   = VIRTMEM_AREA_FLAG_KERNEL|
                    VIRTMEM_AREA_FLAG_IDENTITY|
                    VIRTMEM_AREA_FLAG_POLUTE|
                    VIRTMEM_AREA_FLAG_PAGETABLES},
        {/* user virtual memory: 4 MiB - 3 GiB */
         .pgindex = 1024,
         .npages  = 785408,
         .flags   = VIRTMEM_AREA_FLAG_USER},
        {/* task state memory: >3 GiB */
         .pgindex = 786432,
         .npages  = PAGE_SPAN(MAXTASK*sizeof(struct task)),
         .flags   = VIRTMEM_AREA_FLAG_KERNEL},
        {/* high kernel temporary virtual memory: >3 GiB */
         .pgindex = 786432+PAGE_SPAN(MAXTASK*sizeof(struct task)),
         .npages  = 1,
         .flags   = VIRTMEM_AREA_FLAG_KERNEL|
                    VIRTMEM_AREA_FLAG_PAGETABLES},
        {/* high kernel virtual memory: >3 GiB */
         .pgindex = 786433+PAGE_SPAN(MAXTASK*sizeof(struct task)),
         .npages  = 262144-PAGE_SPAN(MAXTASK*sizeof(struct task))-1,
         .flags   = VIRTMEM_AREA_FLAG_KERNEL|
                    VIRTMEM_AREA_FLAG_PAGETABLES}
};

static int
virtmem_map_page_frame_at_nopg(struct page_directory *pd,
                                 unsigned long pfindex,
                                 unsigned long pgindex,
                                 unsigned int flags)
{
        unsigned long ptindex;
        int err;
        struct page_table *pt;

        /* get page table */

        ptindex = pagetable_index(page_offset(pgindex));

        pt = pageframe_address(pde_get_pageframe_index(pd->entry[ptindex]));

        if (!pt) {
                /* no page table present */
                err = -2;
                goto err_nopagetable;
        }

        return page_table_map_page_frame(pt, pfindex, pgindex&0x3ff, flags);

err_nopagetable:
        return err;
}

static int
virtmem_map_page_frames_at_nopg(struct page_directory *pd,
                                  unsigned long pfindex,
                                  unsigned long pgindex,
                                  unsigned long count,
                                  unsigned int flags)
{
        int err = 0;

        while (count && !(err < 0)) {
                err = virtmem_map_page_frame_at_nopg(pd, pfindex,
                                                         pgindex,
                                                         flags);
                ++pgindex;
                ++pfindex;
                --count;
        }

        return err;
}

static int
virtmem_alloc_page_table_nopg(struct page_directory *pd, unsigned long ptindex,
                                                    unsigned int flags)
{
        unsigned long pfindex;
        int err;

        if (pde_get_pageframe_index(pd->entry[ptindex])) {
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

        return page_directory_install_page_table(pd, pfindex, ptindex, flags);

err_page_table_init:
        physmem_unref_frames(pfindex, pageframe_count(sizeof(struct page_table)));
err_physmem_alloc_frames:
        return err;
}

static int
virtmem_alloc_page_tables_nopg(struct page_directory *pd, unsigned long ptindex,
                                                     unsigned long ptcount,
                                                     unsigned int flags)
{
        int err;

        for (err = 0; ptcount && !(err < 0); ++ptindex, --ptcount) {
                err = virtmem_alloc_page_table_nopg(pd, ptindex, flags);
        }

        return err;
}

static struct page_table *
virtmem_get_page_table_tmp(void)
{
        return page_address(g_virtmem_area[VIRTMEM_AREA_LOW].pgindex +
                            g_virtmem_area[VIRTMEM_AREA_LOW].npages -
                            g_virtmem_area[VIRTMEM_AREA_KERNEL_TMP].npages);
}

static unsigned long
virtmem_get_page_tmp(size_t i)
{
        return g_virtmem_area[VIRTMEM_AREA_KERNEL_TMP].pgindex + i;
}

static int
virtmem_page_is_tmp(unsigned long pgindex)
{
        return (g_virtmem_area[VIRTMEM_AREA_KERNEL_TMP].pgindex <= pgindex) &&
               (pgindex < (g_virtmem_area[VIRTMEM_AREA_KERNEL_TMP].pgindex +
                           g_virtmem_area[VIRTMEM_AREA_KERNEL_TMP].npages));
}

static unsigned long
virtmem_get_index_tmp(unsigned long pgindex)
{
        return pgindex - g_virtmem_area[VIRTMEM_AREA_KERNEL_TMP].pgindex;
}

int
virtmem_install(struct page_directory *pd)
{
        int err;
        const struct virtmem_area *beg, *end;

        beg = g_virtmem_area;
        end = beg + sizeof(g_virtmem_area)/sizeof(g_virtmem_area[0]);

        /* install page tables in all kernel areas */

        for (err = 0; (beg < end) && !(err < 0); ++beg) {

                unsigned long ptindex, ptcount;

                if (!(beg->flags&VIRTMEM_AREA_FLAG_PAGETABLES)) {
                        continue;
                }

                ptindex = pagetable_index(page_offset(beg->pgindex));
                ptcount = pagetable_count(page_offset(beg->pgindex),
                                          page_memory(beg->npages));

                /* create page tables for low area */

                err = virtmem_alloc_page_tables_nopg(pd,
                                                     ptindex,
                                                     ptcount,
                                                     PDE_FLAG_PRESENT|
                                                     PDE_FLAG_WRITEABLE);
        }

        /* create identity mapping for all identity areas */

        beg = g_virtmem_area;
        end = beg + sizeof(g_virtmem_area)/sizeof(g_virtmem_area[0]);

        for (; (beg < end) && !(err < 0); ++beg) {

                if (!(beg->flags&VIRTMEM_AREA_FLAG_POLUTE)) {
                        continue;
                }

                if (beg->flags&VIRTMEM_AREA_FLAG_IDENTITY) {
                        err = virtmem_map_page_frames_at_nopg(pd,
                                                              beg->pgindex,
                                                              beg->pgindex,
                                                              beg->npages,
                                                              PTE_FLAG_PRESENT|
                                                              PTE_FLAG_WRITEABLE);
                } else {
                        unsigned long pfindex = physmem_alloc_frames(
                                pageframe_count(page_memory(beg->npages)));

                        if (!pfindex) {
                                err = -1;
                                break;
                        }

                        err = virtmem_map_page_frames_at_nopg(pd,
                                                              pfindex,
                                                              beg->pgindex,
                                                              beg->npages,
                                                              PTE_FLAG_PRESENT|
                                                              PTE_FLAG_WRITEABLE);
                }
        }

        if (err < 0) {
                return err;
        }

        /* Hand crafted: lowest page table in kernel memory resides in highest
           page frame of identity-mapped low memory. Allows for temporary
           mappings by writing to low-area page.
         */

        {
                struct page_table *pt;
                unsigned long ptpfindex, index;

                pt = virtmem_get_page_table_tmp();

                if ((err = page_table_init(pt)) < 0) {
                        return err;
                }

                ptpfindex = page_index((unsigned long)pt);

                index = pagetable_index(
                                page_offset(
                                        g_virtmem_area[VIRTMEM_AREA_KERNEL_TMP].pgindex));

                err = page_directory_install_page_table(pd,
                                ptpfindex,
                                index,
                                PTE_FLAG_PRESENT|PTE_FLAG_WRITEABLE);
        }

        return err;
}

static unsigned long
virtmem_install_page_frame_tmp(unsigned long pfindex)
{
        volatile pde_type *ptebeg, *pteend, *pte;
        struct page_table *pt;
        unsigned long pgindex;

        pt = virtmem_get_page_table_tmp();

        /* find empty temporary page */

        ptebeg = pt->entry;
        pteend = pt->entry + sizeof(pt->entry)/sizeof(pt->entry[0]);
        pte = ptebeg;

        while ((pte < pteend) && (pte_get_pageframe_index(*pte))) {
                ++pte;
        }

        if (!(pteend-pte)) {
                /* none found */
                return 0;
        }

        /* establish mapping to page frame */

        physmem_ref_frames(pfindex, 1);

        __asm__("mfence\n\t");

        *pte = pte_create(pfindex, PTE_FLAG_PRESENT|PTE_FLAG_WRITEABLE);

        pgindex = virtmem_get_page_tmp(pte-ptebeg);

        mmu_flush_tlb_entry(page_address(pgindex));

        return pgindex;
}

static unsigned long
virtmem_uninstall_page_tmp(unsigned long pgindex)
{
        struct page_table *pt;
        unsigned long index;
        int err;

        if (!virtmem_page_is_tmp(pgindex)) {
                /* not temporarily mapped */
                return 0;
        }

        pt = virtmem_get_page_table_tmp();

        index = virtmem_get_index_tmp(pgindex);

        __asm__("mfence\n\t");

        /* remove mapping */
        if ((err = page_table_unmap_page_frame(pt, index)) < 0) {
                goto err_page_table_unmap_page_frame;
        }

        mmu_flush_tlb_entry(page_address(pgindex));
        mmu_flush_tlb();

        return 0;

err_page_table_unmap_page_frame:
        return err;
}

static unsigned long
virtmem_check_empty_pages_at(const struct page_directory *pd,
                             unsigned long pgindex,
                             unsigned long pgcount)
{
        unsigned long ptindex, ptcount;
        unsigned long nempty;

        ptindex = pagetable_index(page_offset(pgindex));
        ptcount = pagetable_count(page_offset(pgindex), page_memory(pgcount));

        for (nempty = 0; ptcount; --ptcount, ++ptindex) {

                unsigned int pfindex;

                pfindex = pde_get_pageframe_index(pd->entry[ptindex]);

                if (!pfindex) {
                        nempty += minul(pgcount, 1024-(pgindex&0x3ff));
                } else {

                        unsigned long ptpgindex;
                        const struct page_table *pt;
                        size_t i;

                        /* install page table in virtual address space */

                        ptpgindex = virtmem_install_page_frame_tmp(pfindex);

                        if (!ptpgindex) {
                                break;
                        }

                        pt = page_address(ptpgindex);

                        /* count empty pages */

                        for (i = 0;
                             pgcount
                             && (i < sizeof(pt->entry)/sizeof(pt->entry[0]))
                             && (!pte_get_pageframe_index(pt->entry[i]));
                           ++i) {
                                --pgcount;
                                ++pgindex;
                                ++nempty;
                        }

                        /* uninstall page table */
                        virtmem_uninstall_page_tmp(ptpgindex);
                }
        }

        return nempty;
}

static unsigned long
virtmem_find_empty_pages(const struct page_directory *pd,
                                unsigned long npages,
                                unsigned long pgindex_beg,
                                unsigned long pgindex_end)
{
        /* FIXME: this function subject to integer overflow errors
         */

        /* find continuous area in virtual memory */

        while ((pgindex_beg < pgindex_end)
                && (npages < (pgindex_end-pgindex_beg))) {

                unsigned long nempty;

                nempty = virtmem_check_empty_pages_at(pd, pgindex_beg, npages);

                if (nempty == npages) {
                        return pgindex_beg;
                }

                /* goto page after non-empty one */
                pgindex_beg += nempty+1;
        }

        return 0;
}

static int
virtmem_alloc_pages_at(struct page_directory *pd, unsigned long pgindex,
                                                  unsigned long pgcount,
                                                  unsigned int pteflags)
{
        unsigned long ptindex, ptcount;
        unsigned long i;
        int err;

        ptindex = pagetable_index(page_offset(pgindex));
        ptcount = pagetable_count(page_offset(pgindex), page_memory(pgcount));

        for (err = 0, i = 0; (i < ptcount) && !(err < 0); ++i) {

                unsigned long ptpfindex;
                unsigned long ptpgindex;
                struct page_table *pt;
                size_t j;

                ptpfindex = pde_get_pageframe_index(pd->entry[ptindex+i]);

                if (!ptpfindex) {

                        /* allocate and map page-table page frames */

                        ptpfindex = physmem_alloc_frames(
                                pageframe_count(sizeof(struct page_table)));

                        if (!ptpfindex) {
                                err = -1;
                                break;
                        }

                        ptpgindex = virtmem_install_page_frame_tmp(ptpfindex);

                        if (!ptpgindex) {
                                err = -1;
                                break;
                        }

                        /* init and install page table */

                        pt = page_address(ptpgindex);

                        if ((err = page_table_init(pt)) < 0) {
                                break;
                        }

                        __asm__("mfence\n\t");

                        err = page_directory_install_page_table(pd,
                                                        ptpfindex,
                                                        ptindex+i,
                                                        PDE_FLAG_PRESENT|
                                                        PDE_FLAG_WRITEABLE);
                        if (err < 0) {
                                break;
                        }

                        mmu_flush_tlb();

                } else {

                        /* map page-table page frames */

                        ptpgindex = virtmem_install_page_frame_tmp(ptpfindex);

                        if (!ptpgindex) {
                                err = -1;
                                break;
                        }

                        pt = page_address(ptpgindex);
                }

                /* allocate pages within page table */

                __asm__("mfence\n\t");

                for (j = pgindex&0x3ff;
                     pgcount
                     && (j < sizeof(pt->entry)/sizeof(pt->entry[0]))
                     && !(err < 0);
                   --pgcount, ++j, ++pgindex) {
                        unsigned long pfindex = physmem_alloc_frames(1);

                        if (!pfindex) {
                                err = -1;
                                break;
                        }

                        err = page_table_map_page_frame(pt,
                                                        pfindex, j,
                                                        pteflags);
                        if (err < 0) {
                                break;
                        }
                }

                /* unmap page table */
                virtmem_uninstall_page_tmp(ptpgindex);
        }

        mmu_flush_tlb();

        return err;
}

unsigned long
virtmem_alloc_pages_in_area(struct page_directory *pd, unsigned long npages,
                      const struct virtmem_area *area,
                            unsigned int pteflags)
{
        int err;
        unsigned long pgindex;

        pgindex = virtmem_find_empty_pages(pd, npages, area->pgindex,
                                                       area->npages);
        if (!pgindex) {
                err = -1;
                goto err_page_directory_find_empty_pages;
        }

        err = virtmem_alloc_pages_at(pd, pgindex, npages, pteflags);

        if (err < 0) {
                goto err_page_directory_alloc_pages_at;
        }

        return pgindex;

err_page_directory_alloc_pages_at:
err_page_directory_find_empty_pages:
        return 0;
}

unsigned long
virtmem_lookup_physical_page(const struct page_directory *pd,
                             unsigned long pgindex)
{
        return 0;
}

