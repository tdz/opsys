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
#include <semaphore.h>

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

#include "vmem.h"
#include "vmem_32.h"
#include "vmem_pae.h"

static struct page_table *
vmem_get_page_table_tmp(void)
{
        const struct vmem_area *low, *tmp;

        low = vmem_area_get_by_name(VIRTMEM_AREA_LOW);
        tmp = vmem_area_get_by_name(VIRTMEM_AREA_KERNEL_TMP);

        return page_address(low->pgindex + low->npages - (tmp->npages >> 10));
}

int
vmem_install_tmp(struct vmem *as)
{
        struct page_directory *pd;
        struct page_table *pt;
        int err;
        os_index_t ptpfindex, index;
        const struct vmem_area *tmp;

        pd = as->tlps;

        pt = vmem_get_page_table_tmp();

        if ((err = page_table_init(pt)) < 0)
        {
                goto err_page_table_init;
        }

        ptpfindex = page_index(pt);

        tmp = vmem_area_get_by_name(VIRTMEM_AREA_KERNEL_TMP);

        index = pagetable_index(page_address(tmp->pgindex));

        err = page_directory_install_page_table(pd,
                                                ptpfindex,
                                                index,
                                                PTE_FLAG_PRESENT |
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

int
vmem_init(struct vmem *as,
                   enum paging_mode pgmode, void *tlps)
{
        int err;

        if ((err = semaphore_init(&as->sem, 1)) < 0)
        {
                goto err_semaphore_init;
        }

        as->pgmode = pgmode;
        as->tlps = tlps;

        return 0;

err_semaphore_init:
        return err;
}

void
vmem_uninit(struct vmem *as)
{
        semaphore_uninit(&as->sem);
}

void
vmem_enable(const struct vmem *as)
{
        static void (* const enable[])(const void *tlps) =
        {
                vmem_32_enable,
                vmem_pae_enable
        };

        enable[as->pgmode](as->tlps);
}

static int
vmem_map_pageframe_nopg(struct vmem *as,
                                 os_index_t pfindex,
                                 os_index_t pgindex, unsigned int flags)
{
        static int (* const map_pageframe_nopg[])(void *,
                                                  os_index_t,
                                                  os_index_t, unsigned int) =
        {
                vmem_32_map_pageframe_nopg,
                vmem_pae_map_pageframe_nopg
        };

        return map_pageframe_nopg[as->pgmode] (as->tlps,
                                               pgindex, pgindex, flags);
}

int
vmem_map_pageframes_nopg(struct vmem *as,
                                  os_index_t pfindex,
                                  os_index_t pgindex,
                                  size_t count, unsigned int flags)
{
        int err = 0;

        while (count && !(err < 0))
        {
                err = vmem_map_pageframe_nopg(as, pfindex,
                                                       pgindex, flags);
                ++pgindex;
                ++pfindex;
                --count;
        }

        return err;
}

static int
vmem_alloc_page_table_nopg(struct vmem *as,
                                    os_index_t ptindex, unsigned int flags)
{
        static int (*const alloc_page_table_pg[]) (void *,
                                                   os_index_t, unsigned int) =
        {
                vmem_32_alloc_page_table_nopg,
                vmem_pae_alloc_page_table_nopg
        };

        return alloc_page_table_pg[as->pgmode] (as->tlps, ptindex, flags);
}

int
vmem_alloc_page_tables_nopg(struct vmem *as,
                                     os_index_t ptindex,
                                     size_t ptcount, unsigned int flags)
{
        int err;

        for (err = 0; ptcount && !(err < 0); ++ptindex, --ptcount)
        {
                err = vmem_alloc_page_table_nopg(as, ptindex, flags);
        }

        return err;
}

int
vmem_alloc_frames(struct vmem *as, os_index_t pfindex, os_index_t pgindex,
                  size_t pgcount, unsigned int pteflags)
{
        static int (*const alloc_frames[])(void *,
                                               os_index_t,
                                               os_index_t,
                                               size_t, unsigned int) =
        {
                vmem_32_alloc_frames,
                vmem_pae_alloc_frames
        };

        return alloc_frames[as->pgmode](as->tlps, pfindex,
                                        pgindex, pgcount, pteflags);
}

os_index_t
vmem_lookup_frame(const struct vmem * as, os_index_t pgindex)
{
        static os_index_t (*const lookup_frame[])(const void *, os_index_t) =
        {
                vmem_32_lookup_frame,
                vmem_pae_lookup_frame
        };

        return lookup_frame[as->pgmode] (as->tlps, pgindex);
}

size_t
vmem_check_empty_pages(const struct vmem * as,
                                os_index_t pgindex, size_t pgcount)
{
        static size_t(*const check_empty[]) (const void *,
                                             os_index_t, size_t) =
        {
                vmem_32_check_empty_pages,
                vmem_pae_check_empty_pages
        };

        return check_empty[as->pgmode](as->tlps, pgindex, pgcount);
}

os_index_t
vmem_find_empty_pages(const struct vmem * as, size_t npages,
                      os_index_t pgindex_beg, os_index_t pgindex_end)
{
        /*
         * find continuous area in virtual memory 
         */

        while ((pgindex_beg < pgindex_end)
               && (npages < (pgindex_end - pgindex_beg)))
        {
                size_t nempty;

                nempty = vmem_check_empty_pages(as, pgindex_beg, npages);

                if (nempty == npages)
                {
                        return pgindex_beg;
                }

                /*
                 * goto page after non-empty one 
                 */
                pgindex_beg += nempty + 1;
        }

        return -ENOMEM;
}

int
vmem_alloc_pages(struct vmem *as,
                          os_index_t pgindex,
                          size_t pgcount, unsigned int pteflags)
{
        static int (*const alloc_pages[]) (void *,
                                           os_index_t, size_t, unsigned int) =
        {
                vmem_32_alloc_pages,
                vmem_pae_alloc_pages
        };

        return alloc_pages[as->pgmode] (as->tlps, pgindex, pgcount, pteflags);
}

int
vmem_map_pages(struct vmem *dst_as,
                        os_index_t dst_pgindex,
                        const struct vmem *src_as,
                        os_index_t src_pgindex,
                        size_t pgcount, unsigned long pteflags)
{
        static int (*const map_pages[]) (void *,
                                         os_index_t,
                                         const struct vmem *,
                                         os_index_t, size_t, unsigned long) =
        {
                vmem_32_map_pages,
                vmem_pae_map_pages
        };

        return map_pages[dst_as->pgmode] (dst_as->tlps,
                                          dst_pgindex,
                                          src_as,
                                          src_pgindex, pgcount, pteflags);
}

int
vmem_share_2nd_lvl_ps(struct vmem *dst_as,
                               const struct vmem *src_as,
                               os_index_t pgindex, size_t pgcount)
{
        static int (*const share_2nd_lvl_ps[]) (void *,
                                                const void *, os_index_t,
                                                size_t) =
        {
                vmem_32_share_2nd_lvl_ps,
                vmem_pae_share_2nd_lvl_ps
        };

        return share_2nd_lvl_ps[dst_as->pgmode] (dst_as->tlps,
                                                 src_as->tlps,
                                                 pgindex, pgcount);
}
