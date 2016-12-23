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
#include <stddef.h>
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

static struct page_table*
alloc_and_init_page_table(struct vmem_32* vmem32, unsigned long i)
{
    unsigned long pfcount = pageframe_count(sizeof(struct page_table));

    /* allocate and map page-table page frames */

    os_index_t pfindex = pmem_alloc_frames(pfcount);

    if (!pfindex) {
        return NULL;
    }

    os_index_t pgindex = vmem_32_install_page_frame_tmp(pfindex);

    if (pgindex < 0) {
        goto err_vmem_32_install_page_frame_tmp;
    }

    /* init and install page table */

    struct page_table* pt = page_address(pgindex);

    if (!pt) {
        goto err_page_address;
    }

    int res = page_table_init(pt);
    if (res < 0) {
        goto err_page_table_init;
    }

    res = page_directory_install_page_table(vmem32->pd,
                                            pfindex,
                                            i,
                                            PDE_FLAG_PRESENT |
                                            PDE_FLAG_WRITEABLE);
    if (res < 0) {
        goto err_page_directory_install_page_table;
    }

    mmu_flush_tlb();

    return pt;

err_page_directory_install_page_table:
err_page_table_init:
err_page_address:
    vmem_32_uninstall_page_tmp(pgindex);
err_vmem_32_install_page_frame_tmp:
    pmem_unref_frames(pfindex, pfcount);
    return NULL;
}

static struct page_table*
map_page_table(struct vmem_32* vmem32, unsigned long i, bool init_if_none)
{
    os_index_t pfindex = pde_get_pageframe_index(vmem32->pd->entry[i]);

    if (!pfindex && init_if_none) {
        return alloc_and_init_page_table(vmem32, i);
    }

    /* map page-table page frames */
    os_index_t pgindex = vmem_32_install_page_frame_tmp(pfindex);

    if (pgindex < 0) {
        return NULL;
    }

    struct page_table* pt = page_address(pgindex);

    if (!pt) {
        goto err_page_address;
    }

    return pt;

err_page_address:
    vmem_32_uninstall_page_tmp(pgindex);
    return NULL;
}

static void
unmap_page_table(struct vmem_32* vmem32, struct page_table* pt)
{
    vmem_32_uninstall_page_tmp(page_index(pt));
}

/*
 * Public functions
 */

int
vmem_32_init(struct vmem_32* vmem32, struct page_directory* pd)
{
    vmem32->pd = pd;

    return 0;
}

void
vmem_32_uninit(struct vmem_32* vmem32)
{ }

void
vmem_32_enable(struct vmem_32* vmem32)
{
        const struct page_directory *pd = vmem32->pd;

        mmu_load(((unsigned long)pd->entry) & (~0xfff));
        mmu_enable_paging();
}

size_t
vmem_32_check_empty_pages(struct vmem_32* vmem32, os_index_t pgindex, size_t pgcount)
{
        os_index_t ptindex;
        size_t ptcount, nempty;

        ptindex = pagetable_index(page_address(pgindex));
        ptcount = pagetable_count(page_address(pgindex), page_memory(pgcount));

        for (nempty = 0; ptcount; --ptcount, ++ptindex)
        {
                struct page_table* pt = map_page_table(vmem32, ptindex, false);

                if (!pt) {
                        nempty +=
                                minul(pgcount,
                                      1024 - pagetable_page_index(pgindex));
                } else {
                        /* count empty pages at beginning of page table */
                        for (size_t i = pagetable_page_index(pgindex);
                                pgcount
                                && (i < ARRAY_NELEMS(pt->entry))
                                && (!pte_get_pageframe_index(pt->entry[i]));
                             ++i)
                        {
                                --pgcount;
                                ++pgindex;
                                ++nempty;
                        }

                        unmap_page_table(vmem32, pt);
                }
        }

        return nempty;
}

int
vmem_32_alloc_frames(struct vmem_32* vmem32, os_index_t pfindex, os_index_t pgindex,
                     size_t pgcount, unsigned int pteflags)
{
        os_index_t ptindex;
        size_t ptcount, i;
        int err;

        ptindex = pagetable_index(page_address(pgindex));
        ptcount = pagetable_count(page_address(pgindex), page_memory(pgcount));

        err = 0;

        for (i = 0; (i < ptcount) && !(err < 0); ++i)
        {
                struct page_table *pt = map_page_table(vmem32, ptindex + i, true);

                if (!pt) {
                        err = -ENOMEM;
                        break;
                }

                for (size_t j = pagetable_page_index(pgindex);
                     pgcount
                     && (j < ARRAY_NELEMS(pt->entry))
                     && !(err < 0); --pgcount, ++j, ++pgindex, ++pfindex)
                {
                        err = page_table_map_page_frame(pt,
                                                        pfindex,
                                                        j,
                                                        pteflags);
                        if (err < 0)
                        {
                                break;
                        }
                }

                unmap_page_table(vmem32, pt);
        }

        mmu_flush_tlb();

        return err;
}

os_index_t
vmem_32_lookup_frame(struct vmem_32* vmem32, os_index_t pgindex)
{
        os_index_t pfindex;
        int err;

        os_index_t ptindex = pagetable_index(page_address(pgindex));

        struct page_table* pt = map_page_table(vmem32, ptindex, false);

        if (!pt) {
            err = -EFAULT;
            goto err_map_page_table;
        }

        if (!pte_is_present(pt->entry[pgindex & 0x3ff]))
        {
                err = -EFAULT;
                goto err_pte_is_present;
        }

        pfindex = pte_get_pageframe_index(pt->entry[pgindex & 0x3ff]);

        unmap_page_table(vmem32, pt);

        return pfindex;

err_pte_is_present:
        unmap_page_table(vmem32, pt);
err_map_page_table:
        return err;
}

int
vmem_32_alloc_pages(struct vmem_32* vmem32, os_index_t pgindex, size_t pgcount,
                    unsigned int pteflags)
{
        os_index_t ptindex;
        size_t ptcount;
        size_t i;
        int err;

        ptindex = pagetable_index(page_address(pgindex));
        ptcount = pagetable_count(page_address(pgindex), page_memory(pgcount));

        err = 0;

        for (i = 0; (i < ptcount) && !(err < 0); ++i)
        {
            struct page_table* pt = map_page_table(vmem32, ptindex + i, true);

            if (!pt) {
                err = -EFAULT;
                break;
            }

            /* allocate pages within page table */

            for (size_t j = pagetable_page_index(pgindex);
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
            unmap_page_table(vmem32, pt);
        }

        mmu_flush_tlb();

        return err;
}

int
vmem_32_map_pages(struct vmem_32* dst_as, os_index_t dst_pgindex,
                  struct vmem_32 *src_as, os_index_t src_pgindex,
                  size_t pgcount, unsigned long pteflags)
{
        os_index_t dst_ptindex, dst_ptcount;
        int err;
        os_index_t i;

        dst_ptindex = pagetable_index(page_address(dst_pgindex));
        dst_ptcount = pagetable_count(page_address(dst_pgindex),
                                      page_memory(pgcount));

        err = 0;

        for (i = 0; (i < dst_ptcount) && !(err < 0); ++i)
        {
            struct page_table* dst_pt = map_page_table(dst_as, dst_ptindex + i, true);

            if (!dst_pt) {
                err = -EFAULT;
                break;
            }

            /* map pages within page table */

            for (size_t j = pagetable_page_index(dst_pgindex);
                     pgcount
                     && (j < ARRAY_NELEMS(dst_pt->entry))
                     && !(err < 0); --pgcount, ++j, ++dst_pgindex)
            {
                    os_index_t src_pfindex;

                    src_pfindex = vmem_32_lookup_frame(src_as, src_pgindex);

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

            unmap_page_table(dst_as, dst_pt);
        }

        mmu_flush_tlb();

        return err;
}

int
vmem_32_share_2nd_lvl_ps(struct vmem_32* dst_vmem32,
                         struct vmem_32* src_vmem32,
                         os_index_t pgindex, size_t pgcount)
{
        struct page_directory *dst_pd;
        const struct page_directory *src_pd;
        os_index_t ptindex;
        size_t ptcount;

        dst_pd = dst_vmem32->pd;
        src_pd = src_vmem32->pd;

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

/*
 * Public functions for Protected Mode setup
 */

int
vmem_32_map_pageframe_nopg(struct vmem_32* vmem32, os_index_t pfindex,
                           os_index_t pgindex, unsigned int flags)
{
    struct page_directory* pd = vmem32->pd;

    /* get page table */

    os_index_t ptindex = pagetable_index(page_address(pgindex));

    struct page_table* pt =
        pageframe_address(pde_get_pageframe_index(pd->entry[ptindex]));

    if (!pt) {
        // no page table present
        return -ENOMEM;
    }

    int res = page_table_map_page_frame(pt,
                                        pfindex,
                                        pagetable_page_index(pgindex),
                                        flags);
    if (res < 0) {
        goto err_page_table_map_page_frame;
    }

err_page_table_map_page_frame:
    return res;
}

int
vmem_32_alloc_page_table_nopg(struct vmem_32* vmem32, os_index_t ptindex,
                              unsigned int flags)
{
    struct page_directory* pd = vmem32->pd;

    if (pde_get_pageframe_index(pd->entry[ptindex])) {
        return 0;       // page table already exists
    }

    os_index_t pfindex =
        pmem_alloc_frames(pageframe_count(sizeof(struct page_table)));

    if (!pfindex) {
        return -ENOMEM;
    }

    struct page_table* pt = pageframe_address(pfindex);

    int res = page_table_init(pt);

    if (res < 0) {
        goto err_page_table_init;
    }

    res = page_directory_install_page_table(pd, pfindex, ptindex, flags);

    if (res < 0) {
        goto err_page_directory_install_page_table;
    }

    return 0;

err_page_directory_install_page_table:
    page_table_uninit(pt);
err_page_table_init:
    pmem_unref_frames(pfindex, pageframe_count(sizeof(struct page_table)));
    return res;
}

int
vmem_32_install_tmp_nopg(struct vmem_32* vmem32)
{
    struct page_directory* pd = vmem32->pd;

    struct page_table* pt = vmem_32_get_page_table_tmp();

    int res = pmem_claim_frames(pageframe_index(pt), 1);

    if (res < 0) {
        return res;
    }

    res = page_table_init(pt);
    if (res < 0) {
        goto err_page_table_init;
    }

    os_index_t ptpfindex = page_index(pt);

    const struct vmem_area* tmp = vmem_area_get_by_name(VMEM_AREA_TMPMAP);

    os_index_t index = pagetable_index(page_address(tmp->pgindex));

    res = page_directory_install_page_table(pd, ptpfindex, index,
                                            PTE_FLAG_PRESENT |
                                            PTE_FLAG_WRITEABLE);
    if (res < 0) {
        goto err_page_directory_install_page_table;
    }

    // map page-table page frame into kernel address space
    res = vmem_32_map_pageframe_nopg(vmem32, ptpfindex, ptpfindex,
                                     PTE_FLAG_PRESENT |
                                     PTE_FLAG_WRITEABLE);
    if (res < 0) {
        goto err_vmem_32_map_pageframe_nopg;
    }

    return 0;

err_vmem_32_map_pageframe_nopg:
    page_directory_uninstall_page_table(pd, index);
err_page_directory_install_page_table:
    page_table_uninit(pt);
err_page_table_init:
    pmem_unref_frames(pageframe_index(pt), 1);
    return res;
}
