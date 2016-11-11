/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2010  Thomas Zimmermann
 *  Copyright (C) 2016  Thomas Zimmermann
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

#include "vmem_32.h"
#include <arch/i386/page.h>
#include <errno.h>
#include <string.h>
#include "membar.h"
#include "minmax.h"
#include "mmu.h"
#include "pagedir.h"
#include "pageframe.h"
#include "pagetbl.h"
#include "pmem.h"
#include "vmem.h"
#include "vmemarea.h"

static struct page_table *
vmem_32_get_page_table_tmp(void)
{
        const struct vmem_area *low, *tmp;

        low = vmem_area_get_by_name(VMEM_AREA_KERNEL_LOW);
        tmp = vmem_area_get_by_name(VMEM_AREA_TMPMAP);

        return page_address(low->pgindex + low->npages - (tmp->npages >> 10));
}

static os_index_t
vmem_32_get_page_tmp(size_t i)
{
        const struct vmem_area *tmp =
                vmem_area_get_by_name(VMEM_AREA_TMPMAP);

        return tmp->pgindex + i;
}

static int
vmem_32_page_is_tmp(os_index_t pgindex)
{
        const struct vmem_area *tmp =
                vmem_area_get_by_name(VMEM_AREA_TMPMAP);

        return vmem_area_contains_page(tmp, pgindex);
}

static os_index_t
vmem_32_get_index_tmp(os_index_t pgindex)
{
        const struct vmem_area *tmp =
                vmem_area_get_by_name(VMEM_AREA_TMPMAP);

        return pgindex - tmp->pgindex;
}

static os_index_t
vmem_32_install_page_frame_tmp(os_index_t pfindex)
{
        volatile pde_type *ptebeg, *pteend, *pte;
        struct page_table *pt;
        os_index_t pgindex;

        pt = vmem_32_get_page_table_tmp();

        /*
         * find empty temporary page
         */

        ptebeg = pt->entry;
        pteend = pt->entry + ARRAY_NELEMS(pt->entry);
        pte = ptebeg;

        while ((pte < pteend) && (pte_get_pageframe_index(*pte)))
        {
                ++pte;
        }

        if (!(pteend - pte))
        {
                /*
                 * none found
                 */
                return -EBUSY;
        }

        /*
         * establish mapping to page frame
         */

        pmem_ref_frames(pfindex, 1);

        *pte = pte_create(pfindex, PTE_FLAG_PRESENT|PTE_FLAG_WRITEABLE);

        pgindex = vmem_32_get_page_tmp(pte - ptebeg);

        mmu_flush_tlb_entry(page_address(pgindex));

        return pgindex;
}

static os_index_t
vmem_32_uninstall_page_tmp(os_index_t pgindex)
{
        struct page_table *pt;
        os_index_t index;
        int err;

        if (!vmem_32_page_is_tmp(pgindex))
        {
                /*
                 * not temporarily mapped
                 */
                return -EINVAL;
        }

        pt = vmem_32_get_page_table_tmp();

        index = vmem_32_get_index_tmp(pgindex);

        /*
         * finish access before unmapping page
         */
        rwmembar();

        /*
         * remove mapping
         */
        if ((err = page_table_unmap_page_frame(pt, index)) < 0)
        {
                goto err_page_table_unmap_page_frame;
        }

        mmu_flush_tlb_entry(page_address(pgindex));
        mmu_flush_tlb();

        return 0;

err_page_table_unmap_page_frame:
        return err;
}

int
vmem_32_map_pageframe_nopg(void *tlps, os_index_t pfindex,
                           os_index_t pgindex, unsigned int flags)
{
        struct page_directory *pd;
        os_index_t ptindex;
        int err;
        struct page_table *pt;

        pd = tlps;

        /*
         * get page table
         */

        ptindex = pagetable_index(page_address(pgindex));

        pt = pageframe_address(pde_get_pageframe_index(pd->entry[ptindex]));

        if (!pt)
        {
                /*
                 * no page table present
                 */
                err = -ENOMEM;
                goto err_nopagetable;
        }

        return page_table_map_page_frame(pt,
                                         pfindex,
                                         pagetable_page_index(pgindex),
                                         flags);

err_nopagetable:
        return err;
}

int
vmem_32_alloc_page_table_nopg(void *tlps, os_index_t ptindex,
                              unsigned int flags)
{
        struct page_directory *pd;
        os_index_t pfindex;
        int err;

        pd = tlps;

        if (pde_get_pageframe_index(pd->entry[ptindex]))
        {
                return 0;       /* page table already exists */
        }

        pfindex = pmem_alloc_frames(pageframe_count(sizeof(struct page_table)));

        if (!pfindex)
        {
                err = -ENOMEM;
                goto err_pmem_alloc_frames;
        }

        if ((err = page_table_init(pageframe_address(pfindex))) < 0)
        {
                goto err_page_table_init;
        }

        return page_directory_install_page_table(pd, pfindex, ptindex, flags);

err_page_table_init:
        pmem_unref_frames(pfindex, pageframe_count(sizeof(struct page_table)));
err_pmem_alloc_frames:
        return err;
}

int
vmem_32_install_tmp_nopg(void *tlps)
{
        struct page_directory *pd;
        struct page_table *pt;
        int err;
        os_index_t ptpfindex, index;
        const struct vmem_area *tmp;

        pd = tlps;

        pt = vmem_32_get_page_table_tmp();

        if ((err = page_table_init(pt)) < 0)
        {
                goto err_page_table_init;
        }

        ptpfindex = page_index(pt);

        tmp = vmem_area_get_by_name(VMEM_AREA_TMPMAP);

        index = pagetable_index(page_address(tmp->pgindex));

        err = page_directory_install_page_table(pd,
                                                ptpfindex,
                                                index,
                                                PTE_FLAG_PRESENT|
                                                PTE_FLAG_WRITEABLE);
        if (err < 0)
        {
                goto err_page_directory_install_page_table;
        }

err_page_directory_install_page_table:
        page_table_uninit(pt);
err_page_table_init:
        return err;
}

void
vmem_32_enable(const void *tlps)
{
        const struct page_directory *pd = tlps;

        mmu_load(((unsigned long)pd->entry) & (~0xfff));
        mmu_enable_paging();
}

size_t
vmem_32_check_empty_pages(const void *tlps, os_index_t pgindex, size_t pgcount)
{
        const struct page_directory *pd;
        os_index_t ptindex;
        size_t ptcount, nempty;

        pd = tlps;

        ptindex = pagetable_index(page_address(pgindex));
        ptcount = pagetable_count(page_address(pgindex), page_memory(pgcount));

        for (nempty = 0; ptcount; --ptcount, ++ptindex)
        {
                os_index_t pfindex;

                pfindex = pde_get_pageframe_index(pd->entry[ptindex]);

                if (!pfindex)
                {
                        nempty +=
                                minul(pgcount,
                                      1024 - pagetable_page_index(pgindex));
                }
                else
                {
                        os_index_t ptpgindex;
                        const struct page_table *pt;
                        size_t i;

                        /*
                         * install page table in virtual address space
                         */

                        ptpgindex = vmem_32_install_page_frame_tmp(pfindex);

                        if (ptpgindex < 0)
                        {
                                break;
                        }

                        pt = page_address(ptpgindex);

                        /*
                         * count empty pages
                         */

                        for (i = pagetable_page_index(pgindex);
                             pgcount
                             && (i < ARRAY_NELEMS(pt->entry))
                             && (!pte_get_pageframe_index(pt->entry[i])); ++i)
                        {
                                --pgcount;
                                ++pgindex;
                                ++nempty;
                        }

                        /*
                         * uninstall page table
                         */
                        vmem_32_uninstall_page_tmp(ptpgindex);
                }
        }

        return nempty;
}

int
vmem_32_alloc_frames(void *tlps, os_index_t pfindex, os_index_t pgindex,
                     size_t pgcount, unsigned int pteflags)
{
        struct page_directory *pd;
        os_index_t ptindex;
        size_t ptcount, i;
        int err;

        pd = tlps;

        ptindex = pagetable_index(page_address(pgindex));
        ptcount = pagetable_count(page_address(pgindex), page_memory(pgcount));

        err = 0;

        for (i = 0; (i < ptcount) && !(err < 0); ++i)
        {
                os_index_t ptpfindex;
                os_index_t ptpgindex;
                struct page_table *pt;
                size_t j;

                ptpfindex = pde_get_pageframe_index(pd->entry[ptindex + i]);

                if (!ptpfindex)
                {
                        /*
                         * allocate and map page-table page frames
                         */

                        ptpfindex =
                                pmem_alloc_frames(
                                        pageframe_count(
                                                sizeof(struct page_table)));
                        if (!ptpfindex)
                        {
                                err = -ENOMEM;
                                break;
                        }

                        ptpgindex = vmem_32_install_page_frame_tmp(ptpfindex);

                        if (ptpgindex < 0)
                        {
                                err = ptpgindex;
                                break;
                        }

                        /*
                         * init and install page table
                         */

                        pt = page_address(ptpgindex);

                        if ((err = page_table_init(pt)) < 0)
                        {
                                break;
                        }

                        err = page_directory_install_page_table(pd,
                                                                ptpfindex,
                                                                ptindex + i,
                                                                PDE_FLAG_PRESENT|
                                                                PDE_FLAG_WRITEABLE);
                        if (err < 0)
                        {
                                break;
                        }

                        mmu_flush_tlb();
                }
                else
                {
                        /*
                         * map page-table page frames
                         */

                        ptpgindex = vmem_32_install_page_frame_tmp(ptpfindex);

                        if (ptpgindex < 0)
                        {
                                err = ptpgindex;
                                break;
                        }

                        pt = page_address(ptpgindex);
                }

                /*
                 * allocate pages within page table
                 */

                for (j = pagetable_page_index(pgindex);
                     pgcount
                     && (j < ARRAY_NELEMS(pt->entry))
                     && !(err < 0); --pgcount, ++j, ++pgindex, ++pfindex)
                {
                        err = page_table_map_page_frame(pt, pfindex,
                                                        j, pteflags);
                        if (err < 0)
                        {
                                break;
                        }
                }

                /*
                 * unmap page table
                 */
                vmem_32_uninstall_page_tmp(ptpgindex);
        }

        mmu_flush_tlb();

        return err;
}

os_index_t
vmem_32_lookup_frame(const void *tlps, os_index_t pgindex)
{
        const struct page_directory *pd;
        os_index_t ptindex;
        os_index_t ptpgindex;
        os_index_t ptpfindex;
        struct page_table *pt;
        os_index_t pfindex;
        int err;

        pd = tlps;

        ptindex = pagetable_index(page_address(pgindex));

        ptpfindex = pde_get_pageframe_index(pd->entry[ptindex]);

        /*
         * map page table of page address
         */

        ptpgindex = vmem_32_install_page_frame_tmp(ptpfindex);

        if (ptpgindex < 0)
        {
                err = ptpgindex;
                goto err_vmem_32_install_page_frame_tmp;
        }

        /*
         * read page frame
         */

        pt = page_address(ptpgindex);

        if (!pte_is_present(pt->entry[pgindex & 0x3ff]))
        {
                err = -EFAULT;
                goto err_pte_is_present;
        }

        pfindex = pte_get_pageframe_index(pt->entry[pgindex & 0x3ff]);

        /*
         * unmap page table
         */
        vmem_32_uninstall_page_tmp(ptpgindex);

        return pfindex;

err_pte_is_present:
        vmem_32_uninstall_page_tmp(ptpgindex);
err_vmem_32_install_page_frame_tmp:
        return err;
}

int
vmem_32_alloc_pages(void *tlps, os_index_t pgindex, size_t pgcount,
                    unsigned int pteflags)
{
        struct page_directory *pd;
        os_index_t ptindex;
        size_t ptcount;
        size_t i;
        int err;

        pd = tlps;

        ptindex = pagetable_index(page_address(pgindex));
        ptcount = pagetable_count(page_address(pgindex), page_memory(pgcount));

        err = 0;

        for (i = 0; (i < ptcount) && !(err < 0); ++i)
        {
                os_index_t ptpfindex;
                os_index_t ptpgindex;
                struct page_table *pt;
                size_t j;

                ptpfindex = pde_get_pageframe_index(pd->entry[ptindex + i]);

                if (!ptpfindex)
                {
                        /*
                         * allocate and map page-table page frames
                         */

                        ptpfindex =
                                pmem_alloc_frames(
                                        pageframe_count(
                                                sizeof(struct page_table)));
                        if (!ptpfindex)
                        {
                                err = -ENOMEM;
                                break;
                        }

                        ptpgindex = vmem_32_install_page_frame_tmp(ptpfindex);

                        if (ptpgindex < 0)
                        {
                                err = ptpgindex;
                                break;
                        }

                        /*
                         * init and install page table
                         */

                        pt = page_address(ptpgindex);

                        if ((err = page_table_init(pt)) < 0)
                        {
                                break;
                        }

                        err = page_directory_install_page_table(pd,
                                                                ptpfindex,
                                                                ptindex + i,
                                                                PDE_FLAG_PRESENT|
                                                                PDE_FLAG_WRITEABLE);
                        if (err < 0)
                        {
                                break;
                        }

                        mmu_flush_tlb();
                }
                else
                {
                        /*
                         * map page-table page frames
                         */

                        ptpgindex = vmem_32_install_page_frame_tmp(ptpfindex);

                        if (ptpgindex < 0)
                        {
                                err = ptpgindex;
                                break;
                        }

                        pt = page_address(ptpgindex);
                }

                /*
                 * allocate pages within page table
                 */

                for (j = pagetable_page_index(pgindex);
                     pgcount
                     && (j < ARRAY_NELEMS(pt->entry))
                     && !(err < 0); --pgcount, ++j, ++pgindex)
                {
                        os_index_t pfindex = pmem_alloc_frames(1);

                        if (!pfindex)
                        {
                                err = -ENOMEM;
                                break;
                        }

                        err = page_table_map_page_frame(pt, pfindex, j,
                                                        pteflags);
                        if (err < 0)
                        {
                                break;
                        }
                }

                /*
                 * unmap page table
                 */
                vmem_32_uninstall_page_tmp(ptpgindex);
        }

        mmu_flush_tlb();

        return err;
}

int
vmem_32_map_pages(void *dst_tlps, os_index_t dst_pgindex,
                  const struct vmem *src_as, os_index_t src_pgindex,
                  size_t pgcount, unsigned long pteflags)
{
        os_index_t dst_ptindex, dst_ptcount;
        int err;
        os_index_t i;
        struct page_directory *dst_pd;

        dst_pd = dst_tlps;
        dst_ptindex = pagetable_index(page_address(dst_pgindex));
        dst_ptcount = pagetable_count(page_address(dst_pgindex),
                                      page_memory(pgcount));

        err = 0;

        for (i = 0; (i < dst_ptcount) && !(err < 0); ++i)
        {
                os_index_t ptpfindex;
                os_index_t ptpgindex;
                struct page_table *dst_pt;
                size_t j;

                ptpfindex =
                        pde_get_pageframe_index(dst_pd->entry[dst_ptindex+i]);

                if (!ptpfindex)
                {
                        /*
                         * allocate and map page-table page frames
                         */

                        ptpfindex =
                                pmem_alloc_frames(
                                        pageframe_count(
                                                sizeof(struct page_table)));
                        if (!ptpfindex)
                        {
                                err = -ENOMEM;
                                break;
                        }

                        ptpgindex = vmem_32_install_page_frame_tmp(ptpfindex);

                        if (ptpgindex < 0)
                        {
                                err = ptpgindex;
                                break;
                        }

                        /*
                         * init and install page table
                         */

                        dst_pt = page_address(ptpgindex);

                        if ((err = page_table_init(dst_pt)) < 0)
                        {
                                break;
                        }

                        err = page_directory_install_page_table(dst_pd,
                                                                ptpfindex,
                                                                dst_ptindex + i,
                                                                PDE_FLAG_PRESENT |
                                                                PDE_FLAG_WRITEABLE);
                        if (err < 0)
                        {
                                break;
                        }

                        mmu_flush_tlb();
                }
                else
                {
                        /*
                         * map page-table page frames
                         */

                        ptpgindex = vmem_32_install_page_frame_tmp(ptpfindex);

                        if (ptpgindex < 0)
                        {
                                err = ptpgindex;
                                break;
                        }

                        dst_pt = page_address(ptpgindex);
                }

                /*
                 * map pages within page table
                 */

                for (j = pagetable_page_index(dst_pgindex);
                     pgcount
                     && (j < ARRAY_NELEMS(dst_pt->entry))
                     && !(err < 0); --pgcount, ++j, ++dst_pgindex)
                {
                        os_index_t src_pfindex;

                        src_pfindex = __vmem_lookup_frame(src_as, src_pgindex);

                        if (src_pfindex < 0)
                        {
                                err = src_pfindex;
                                /*
                                 * handle error
                                 */
                        }

                        err = page_table_map_page_frame(dst_pt, src_pfindex,
                                                        j, pteflags);
                        if (err < 0)
                        {
                                break;
                        }
                }

                /*
                 * unmap page table
                 */
                vmem_32_uninstall_page_tmp(ptpgindex);
        }

        mmu_flush_tlb();

        return err;
}

int
vmem_32_share_2nd_lvl_ps(void *dst_tlps, const void *src_tlps,
                         os_index_t pgindex, size_t pgcount)
{
        struct page_directory *dst_pd;
        const struct page_directory *src_pd;
        os_index_t ptindex;
        size_t ptcount;

        dst_pd = dst_tlps;
        src_pd = src_tlps;

        ptindex = pagetable_index(page_address(pgindex));
        ptcount = pagetable_count(page_address(pgindex), page_memory(pgcount));

        while (ptcount)
        {
                dst_pd->entry[ptindex] = src_pd->entry[ptindex];
                ++ptindex;
                --ptcount;
        }

        return 0;
}

