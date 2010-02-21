/*
 *  opsys - A small, experimental operating system
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

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#include <minmax.h>
#include "membar.h"
#include "mmu.h"
#include "cpu.h"

#include <spinlock.h>

/* physical memory */
#include "pageframe.h"
#include <physmem.h>

/* virtual memory */
#include "page.h"
#include "pte.h"
#include "pagetbl.h"
#include "pde.h"
#include "pagedir.h"
#include <vmemarea.h>

#include "addrspace.h"

static struct page_table *
address_space_get_page_table_tmp(void)
{
        const struct virtmem_area *low, *tmp;
        
        low = virtmem_area_get_by_name(VIRTMEM_AREA_LOW);
        tmp = virtmem_area_get_by_name(VIRTMEM_AREA_KERNEL_TMP);

        return page_address(low->pgindex + low->npages - (tmp->npages>>10));
}

static os_index_t
address_space_get_page_tmp(size_t i)
{
        const struct virtmem_area *tmp =
                virtmem_area_get_by_name(VIRTMEM_AREA_KERNEL_TMP);

        return tmp->pgindex + i;
}

static int
address_space_page_is_tmp(os_index_t pgindex)
{
        const struct virtmem_area *tmp =
                virtmem_area_get_by_name(VIRTMEM_AREA_KERNEL_TMP);

        return virtmem_area_contains_page(tmp, pgindex);
}

static os_index_t
address_space_get_index_tmp(os_index_t pgindex)
{
        const struct virtmem_area *tmp =
                virtmem_area_get_by_name(VIRTMEM_AREA_KERNEL_TMP);

        return pgindex - tmp->pgindex;
}


static os_index_t
address_space_install_page_frame_tmp(os_index_t pfindex)
{
        volatile pde_type *ptebeg, *pteend, *pte;
        struct page_table *pt;
        os_index_t pgindex;

        pt = address_space_get_page_table_tmp();

        /* find empty temporary page */

        ptebeg = pt->entry;
        pteend = pt->entry + sizeof(pt->entry)/sizeof(pt->entry[0]);
        pte = ptebeg;

        while ((pte < pteend) && (pte_get_pageframe_index(*pte))) {
                ++pte;
        }

        if (!(pteend-pte)) {
                /* none found */
                return -EBUSY;
        }

        /* establish mapping to page frame */

        physmem_ref_frames(pfindex, 1);

        *pte = pte_create(pfindex, PTE_FLAG_PRESENT|PTE_FLAG_WRITEABLE);

        pgindex = address_space_get_page_tmp(pte-ptebeg);

        mmu_flush_tlb_entry(page_address(pgindex));

        return pgindex;
}

static os_index_t
address_space_uninstall_page_tmp(os_index_t pgindex)
{
        struct page_table *pt;
        os_index_t index;
        int err;

        if (!address_space_page_is_tmp(pgindex)) {
                /* not temporarily mapped */
                return -EINVAL;
        }

        pt = address_space_get_page_table_tmp();

        index = address_space_get_index_tmp(pgindex);

        /* finish access before unmapping page */
        rwmembar();

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

int
address_space_init(struct address_space *as,
                   enum paging_mode pgmode,
                   void *tlps)
{
        int err;

        if ((err = spinlock_init(&as->lock)) < 0) {
                goto err_spinlock_init;
        }

        as->pgmode = pgmode;
        as->tlps = tlps;

        return 0;

err_spinlock_init:
        return err;
}

void
address_space_uninit(struct address_space *as)
{
        spinlock_uninit(&as->lock);
}

int
address_space_install_tmp(struct address_space *as)
{
        struct page_directory *pd;
        struct page_table *pt;
        int err;
        os_index_t ptpfindex, index;
        const struct virtmem_area *tmp;

        pd = as->tlps;

        pt = address_space_get_page_table_tmp();

        if ((err = page_table_init(pt)) < 0) {
                goto err_page_table_init;
        }

        ptpfindex = page_index(pt);

        tmp = virtmem_area_get_by_name(VIRTMEM_AREA_KERNEL_TMP);

        index = pagetable_index(page_address(tmp->pgindex));

        err = page_directory_install_page_table(pd,
                                                ptpfindex,
                                                index,
                                                PTE_FLAG_PRESENT|
                                                PTE_FLAG_WRITEABLE);
        if (err < 0) {
                goto err_page_directory_install_page_table;
        }

err_page_directory_install_page_table:
        page_table_uninit(pt);
err_page_table_init:
        return err;
}

void
address_space_enable(const struct address_space *as)
{
        switch (as->pgmode) {
                case PAGING_32BIT:
                        {
                                const struct page_directory *pd = as->tlps;

                                mmu_load(((unsigned long)pd->entry)&(~0xfff));
                                mmu_enable_paging();
                        }
                        break;
                case PAGING_PAE:
                        break;
                default:
                        break;
        }
}

static int
address_space_map_pageframe_nopg_32bit(void *tlps,
                                       os_index_t pfindex,
                                       os_index_t pgindex,
                                       unsigned int flags)
{
        struct page_directory *pd;
        os_index_t ptindex;
        int err;
        struct page_table *pt;

        pd = tlps;

        /* get page table */

        ptindex = pagetable_index(page_address(pgindex));

        pt = pageframe_address(pde_get_pageframe_index(pd->entry[ptindex]));

        if (!pt) {
                /* no page table present */
                err = -ENOMEM;
                goto err_nopagetable;
        }

        return page_table_map_page_frame(pt,
                                         pfindex,
                                         pagetable_page_index(pgindex), flags);

err_nopagetable:
        return err;
}

static int
address_space_map_pageframe_nopg_pae(void *tlps,
                                     os_index_t pfindex,
                                     os_index_t pgindex,
                                     unsigned int flags)
{
        return -ENOSYS;
}

static int
address_space_map_pageframe_nopg(struct address_space *as,
                                 os_index_t pfindex,
                                 os_index_t pgindex,
                                 unsigned int flags)
{
        static int (* const map_pageframe_nopg[])(void*,
                                                  os_index_t,
                                                  os_index_t,
                                                  unsigned int) = {
                address_space_map_pageframe_nopg_32bit,
                address_space_map_pageframe_nopg_pae};

        return map_pageframe_nopg[as->pgmode](as->tlps,
                                              pgindex,
                                              pgindex,
                                              flags);
}

int
address_space_map_pageframes_nopg(struct address_space *as,
                                  os_index_t pfindex,
                                  os_index_t pgindex,
                                  size_t count,
                                  unsigned int flags)
{
        int err = 0;

        while (count && !(err < 0)) {
                err = address_space_map_pageframe_nopg(as, pfindex,
                                                           pgindex, flags);
                ++pgindex;
                ++pfindex;
                --count;
        }

        return err;
}

static int
address_space_alloc_page_table_nopg_32bit(void *tlps,
                                          os_index_t ptindex,
                                          unsigned int flags)
{
        struct page_directory *pd;
        os_index_t pfindex;
        int err;

        pd = tlps;

        if (pde_get_pageframe_index(pd->entry[ptindex])) {
                return 0; /* page table already exists */
        }

        pfindex = physmem_alloc_frames(pageframe_count(sizeof(struct page_table)));

        if (!pfindex) {
                err = -ENOMEM;
                goto err_physmem_alloc_frames;
        }

        if ((err = page_table_init(pageframe_address(pfindex))) < 0) {
                goto err_page_table_init;
        }

        return page_directory_install_page_table(pd,
                                                 pfindex,
                                                 ptindex,
                                                 flags);

err_page_table_init:
        physmem_unref_frames(pfindex, pageframe_count(sizeof(struct page_table)));
err_physmem_alloc_frames:
        return err;
}

static int
address_space_alloc_page_table_nopg_pae(void *tlps,
                                        os_index_t ptindex,
                                        unsigned int flags)
{
        return -ENOSYS;
}

static int
address_space_alloc_page_table_nopg(struct address_space *as,
                                    os_index_t ptindex,
                                    unsigned int flags)
{
        static int (* const alloc_page_table_pg[])(void*,
                                                   os_index_t,
                                                   unsigned int) = {
                address_space_alloc_page_table_nopg_32bit,
                address_space_alloc_page_table_nopg_pae};

        return alloc_page_table_pg[as->pgmode](as->tlps, ptindex, flags);
}

int
address_space_alloc_page_tables_nopg(struct address_space *as,
                                     os_index_t ptindex,
                                     size_t ptcount,
                                     unsigned int flags)
{
        int err;

        for (err = 0; ptcount && !(err < 0); ++ptindex, --ptcount) {
                err = address_space_alloc_page_table_nopg(as, ptindex, flags);
        }

        return err;
}

static size_t
address_space_check_empty_pages_32bit(const void *tlps,
                                            os_index_t pgindex,
                                            size_t pgcount)
{
        const struct page_directory *pd;
        os_index_t ptindex;
        size_t ptcount, nempty;

        pd = tlps;

        ptindex = pagetable_index(page_address(pgindex));
        ptcount = pagetable_count(page_address(pgindex), page_memory(pgcount));

        for (nempty = 0; ptcount; --ptcount, ++ptindex) {

                os_index_t pfindex;

                pfindex = pde_get_pageframe_index(pd->entry[ptindex]);

                if (!pfindex) {
                        nempty +=
                                minul(pgcount,
                                      1024-pagetable_page_index(pgindex));
                } else {

                        os_index_t ptpgindex;
                        const struct page_table *pt;
                        size_t i;

                        /* install page table in virtual address space */

                        ptpgindex = address_space_install_page_frame_tmp(pfindex);

                        if (ptpgindex < 0) {
                                break;
                        }

                        pt = page_address(ptpgindex);

                        /* count empty pages */

                        for (i = pagetable_page_index(pgindex);
                             pgcount
                             && (i < sizeof(pt->entry)/sizeof(pt->entry[0]))
                             && (!pte_get_pageframe_index(pt->entry[i]));
                           ++i) {
                                --pgcount;
                                ++pgindex;
                                ++nempty;
                        }

                        /* uninstall page table */
                        address_space_uninstall_page_tmp(ptpgindex);
                }
        }

        return nempty;
}

static size_t
address_space_check_empty_pages_pae(const void *tlps,
                                          os_index_t pgindex,
                                          size_t pgcount)
{
        return -ENOSYS;
}

size_t
address_space_check_empty_pages(const struct address_space *as,
                                      os_index_t pgindex,
                                      size_t pgcount)
{
        static size_t (* const check_empty[])(const void*,
                                                    os_index_t,
                                                    size_t) = {
                address_space_check_empty_pages_32bit,
                address_space_check_empty_pages_pae};

        return check_empty[as->pgmode](as->tlps, pgindex, pgcount);
}

os_index_t
address_space_find_empty_pages(const struct address_space *as,
                                     size_t npages,
                                     os_index_t pgindex_beg,
                                     os_index_t pgindex_end)
{
        /* find continuous area in virtual memory */

        while ((pgindex_beg < pgindex_end)
                && (npages < (pgindex_end-pgindex_beg))) {

                size_t nempty;

                nempty = address_space_check_empty_pages(as,
                                                         pgindex_beg,
                                                         npages);
                if (nempty == npages) {
                        return pgindex_beg;
                }

                /* goto page after non-empty one */
                pgindex_beg += nempty+1;
        }

        return -ENOMEM;
}

static int
address_space_alloc_pageframes_32bit(void *tlps,
                                     os_index_t pfindex,
                                     os_index_t pgindex,
                                     size_t pgcount,
                                     unsigned int pteflags)
{
        struct page_directory *pd;
        os_index_t ptindex;
        size_t ptcount, i;
        int err;

        pd = tlps;

        ptindex = pagetable_index(page_address(pgindex));
        ptcount = pagetable_count(page_address(pgindex), page_memory(pgcount));

        err = 0;

        for (i = 0; (i < ptcount) && !(err < 0); ++i) {

                os_index_t ptpfindex;
                os_index_t ptpgindex;
                struct page_table *pt;
                size_t j;

                ptpfindex = pde_get_pageframe_index(pd->entry[ptindex+i]);

                if (!ptpfindex) {

                        /* allocate and map page-table page frames */

                        ptpfindex = physmem_alloc_frames(
                                pageframe_count(sizeof(struct page_table)));

                        if (!ptpfindex) {
                                err = -ENOMEM;
                                break;
                        }

                        ptpgindex = address_space_install_page_frame_tmp(ptpfindex);

                        if (ptpgindex < 0) {
                                err = ptpgindex;
                                break;
                        }

                        /* init and install page table */

                        pt = page_address(ptpgindex);

                        if ((err = page_table_init(pt)) < 0) {
                                break;
                        }

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

                        ptpgindex = address_space_install_page_frame_tmp(ptpfindex);

                        if (ptpgindex < 0) {
                                err = ptpgindex;
                                break;
                        }

                        pt = page_address(ptpgindex);
                }

                /* allocate pages within page table */

                for (j = pagetable_page_index(pgindex);
                     pgcount
                     && (j < sizeof(pt->entry)/sizeof(pt->entry[0]))
                     && !(err < 0);
                   --pgcount, ++j, ++pgindex, ++pfindex) {
                        err = page_table_map_page_frame(pt,
                                                        pfindex, j,
                                                        pteflags);
                        if (err < 0) {
                                break;
                        }
                }

                /* unmap page table */
                address_space_uninstall_page_tmp(ptpgindex);
        }

        mmu_flush_tlb();

        return err;
}

static int
address_space_alloc_pageframes_pae(void *tlps,
                                   os_index_t pfindex,
                                   os_index_t pgindex,
                                   size_t pgcount,
                                   unsigned int pteflags)
{
        return -ENOSYS;
}

int
address_space_alloc_pageframes(struct address_space *as,
                               os_index_t pfindex,
                               os_index_t pgindex,
                               size_t pgcount,
                               unsigned int pteflags)
{
        static int (* const alloc_pageframes[])(void*,
                                                os_index_t,
                                                os_index_t,
                                                size_t,
                                                unsigned int) = {
                address_space_alloc_pageframes_32bit,
                address_space_alloc_pageframes_pae};

        return alloc_pageframes[as->pgmode](as->tlps, pfindex,
                                                      pgindex,
                                                      pgcount,
                                                      pteflags);
}

static os_index_t
address_space_alloc_pages_32bit(void *tlps,
                                os_index_t pgindex,
                                size_t pgcount,
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

        for (i = 0; (i < ptcount) && !(err < 0); ++i) {

                os_index_t ptpfindex;
                os_index_t ptpgindex;
                struct page_table *pt;
                size_t j;

                ptpfindex = pde_get_pageframe_index(pd->entry[ptindex+i]);

                if (!ptpfindex) {

                        /* allocate and map page-table page frames */

                        ptpfindex = physmem_alloc_frames(
                                pageframe_count(sizeof(struct page_table)));

                        if (!ptpfindex) {
                                err = -ENOMEM;
                                break;
                        }

                        ptpgindex = address_space_install_page_frame_tmp(ptpfindex);

                        if (ptpgindex < 0) {
                                err = ptpgindex;
                                break;
                        }

                        /* init and install page table */

                        pt = page_address(ptpgindex);

                        if ((err = page_table_init(pt)) < 0) {
                                break;
                        }

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

                        ptpgindex = address_space_install_page_frame_tmp(ptpfindex);

                        if (ptpgindex < 0) {
                                err = ptpgindex;
                                break;
                        }

                        pt = page_address(ptpgindex);
                }

                /* allocate pages within page table */

                for (j = pagetable_page_index(pgindex);
                     pgcount
                     && (j < sizeof(pt->entry)/sizeof(pt->entry[0]))
                     && !(err < 0);
                   --pgcount, ++j, ++pgindex) {
                        os_index_t pfindex = physmem_alloc_frames(1);

                        if (!pfindex) {
                                err = -ENOMEM;
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
                address_space_uninstall_page_tmp(ptpgindex);
        }

        mmu_flush_tlb();

        return err;
}

static os_index_t
address_space_alloc_pages_pae(void *tlps,
                              os_index_t pgindex,
                              size_t pgcount,
                              unsigned int pteflags)
{
        return -ENOSYS;
}

os_index_t
address_space_alloc_pages(struct address_space *as,
                          os_index_t pgindex,
                          size_t pgcount,
                          unsigned int pteflags)
{
        static os_index_t (* const alloc_pages[])(void*,
                                                  os_index_t,
                                                  size_t,
                                                  unsigned int) = {
                address_space_alloc_pages_32bit,
                address_space_alloc_pages_pae};

        return alloc_pages[as->pgmode](as->tlps, pgindex, pgcount, pteflags);
}

static int
address_space_map_pages_32bit(const struct address_space *src_as,
                              os_index_t src_pgindex,
                              size_t pgcount,
                              void *dst_tlps,
                              os_index_t dst_pgindex,
                              unsigned long dst_pteflags)
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

        for (i = 0; (i < dst_ptcount) && !(err < 0); ++i) {

                os_index_t ptpfindex;
                os_index_t ptpgindex;
                struct page_table *dst_pt;
                size_t j;

                ptpfindex =
                        pde_get_pageframe_index(dst_pd->entry[dst_ptindex+i]);

                if (!ptpfindex) {

                        /* allocate and map page-table page frames */

                        ptpfindex = physmem_alloc_frames(
                                pageframe_count(sizeof(struct page_table)));

                        if (!ptpfindex) {
                                err = -ENOMEM;
                                break;
                        }

                        ptpgindex = address_space_install_page_frame_tmp(ptpfindex);

                        if (ptpgindex < 0) {
                                err = ptpgindex;
                                break;
                        }

                        /* init and install page table */

                        dst_pt = page_address(ptpgindex);

                        if ((err = page_table_init(dst_pt)) < 0) {
                                break;
                        }

                        err = page_directory_install_page_table(dst_pd,
                                                        ptpfindex,
                                                        dst_ptindex+i,
                                                        PDE_FLAG_PRESENT|
                                                        PDE_FLAG_WRITEABLE);
                        if (err < 0) {
                                break;
                        }

                        mmu_flush_tlb();

                } else {

                        /* map page-table page frames */

                        ptpgindex = address_space_install_page_frame_tmp(ptpfindex);

                        if (ptpgindex < 0) {
                                err = ptpgindex;
                                break;
                        }

                        dst_pt = page_address(ptpgindex);
                }

                /* map pages within page table */

                for (j = pagetable_page_index(dst_pgindex);
                     pgcount
                     && (j < sizeof(dst_pt->entry)/sizeof(dst_pt->entry[0]))
                     && !(err < 0);
                   --pgcount, ++j, ++dst_pgindex) {

                        os_index_t src_pfindex;

                        src_pfindex = address_space_lookup_pageframe(src_as,
                                                                     src_pgindex);
                        if (src_pfindex < 0) {
                                err = src_pfindex;
                                /* handle error */
                        }

                        err = page_table_map_page_frame(dst_pt,
                                                        src_pfindex, j,
                                                        dst_pteflags);
                        if (err < 0) {
                                break;
                        }
                }

                /* unmap page table */
                address_space_uninstall_page_tmp(ptpgindex);
        }

        mmu_flush_tlb();

        return err;
}

static int
address_space_map_pages_pae(const struct address_space *src_as,
                                  os_index_t src_pgindex,
                                  size_t pgcount,
                                  void *dst_tlps,
                                  os_index_t dst_pgindex,
                                  unsigned long dst_pteflags)
{
        return -ENOSYS;
}

int
address_space_map_pages(const struct address_space *src_as,
                              os_index_t src_pgindex,
                              size_t pgcount,
                              struct address_space *dst_as,
                              os_index_t dst_pgindex,
                              unsigned long dst_pteflags)
{
        static int (* const map_pages[])(const struct address_space*,
                                         os_index_t,
                                         size_t,
                                         void*,
                                         os_index_t,
                                         unsigned long) = {
                address_space_map_pages_32bit,
                address_space_map_pages_pae};

        return map_pages[dst_as->pgmode](src_as,
                                         src_pgindex, pgcount,
                                         dst_as->tlps,
                                         dst_pgindex,
                                         dst_pteflags);
}

static os_index_t
address_space_lookup_pageframe_32bit(const void *tlps, os_index_t pgindex)
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

        /* map page table of page address */

        ptpgindex = address_space_install_page_frame_tmp(ptpfindex);

        if (ptpgindex < 0) {
                err = ptpgindex;
                goto err_address_space_install_page_frame_tmp;
        }

        /* read page frame */

        pt = page_address(ptpgindex);

        if (!pte_is_present(pt->entry[pgindex&0x3ff])) {
                err = -EFAULT;
                goto err_pte_is_present;
        }

        pfindex = pte_get_pageframe_index(pt->entry[pgindex&0x3ff]);

        /* unmap page table */
        address_space_uninstall_page_tmp(ptpgindex);

        return pfindex;

err_pte_is_present:
        address_space_uninstall_page_tmp(ptpgindex);
err_address_space_install_page_frame_tmp:
        return err;
}

static os_index_t
address_space_lookup_pageframe_pae(const void *tlps, os_index_t pgindex)
{
        return -ENOSYS;
}

os_index_t
address_space_lookup_pageframe(const struct address_space *as,
                                     os_index_t pgindex)
{
        static os_index_t (* const lookup_pageframe[])(const void*,
                                                             os_index_t) = {
                address_space_lookup_pageframe_32bit,
                address_space_lookup_pageframe_pae};

        return lookup_pageframe[as->pgmode](as->tlps, pgindex);
}

static int
address_space_share_2nd_lvl_ps_32bit(void *dst_tlps,
                               const void *src_tlps,
                                     os_index_t pgindex,
                                     size_t pgcount)
{
        struct page_directory *dst_pd;
        const struct page_directory *src_pd;
        os_index_t ptindex;
        size_t ptcount;

        dst_pd = dst_tlps;
        src_pd = src_tlps;

        ptindex = pagetable_index(page_address(pgindex));
        ptcount = pagetable_count(page_address(pgindex), page_memory(pgcount));

        while (ptcount) {
                dst_pd->entry[ptindex] = src_pd->entry[ptindex];
                ++ptindex;
                --ptcount;
        }

        return 0;
}

static int
address_space_share_2nd_lvl_ps_pae(void *dst_tlps,
                             const void *src_tlps,
                                   os_index_t pgindex,
                                   size_t pgcount)
{
        return -ENOSYS;
}

int
address_space_share_2nd_lvl_ps(struct address_space *dst_as,
                         const struct address_space *src_as,
                               os_index_t pgindex,
                               size_t pgcount)
{
        static int (* const share_2nd_lvl_ps[])(void*,
                                          const void*, os_index_t, size_t) = {
                address_space_share_2nd_lvl_ps_32bit,
                address_space_share_2nd_lvl_ps_pae};

        return share_2nd_lvl_ps[dst_as->pgmode](dst_as->tlps,
                                                src_as->tlps,
                                                pgindex,
                                                pgcount);
}

