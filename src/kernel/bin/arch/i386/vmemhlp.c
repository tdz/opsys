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
#include <sys/types.h>

#include <spinlock.h>
#include <semaphore.h>

/* physical memory */
#include "pageframe.h"
#include "pmem.h"

/* virtual memory */
#include <arch/i386/page.h>
#include "pte.h"
#include "pagetbl.h"
#include "pde.h"
#include "pagedir.h"
#include "vmemarea.h"

#include "vmem.h"
#include <alloc.h>

#include "vmemhlp.h"

#include <console.h>

int
vmem_helper_init_kernel_vmem(struct vmem *as)
{
        /*
         * TODO: dynamic allocation of page directory needs rework of the
         *       page-frame allocator. It should be possible to specify
         *       position of page frame, (e.g., < 4Mib).
         */
        static struct page_directory kernel_pd;

        struct page_directory *pd;
        int err;
        enum vmem_area_name name;

        /*
         * allocate and init page directory for kernel task 
         */

        /*
         * os_index_t pfindex;
         * 
         * if (!(pfindex = pmem_alloc_frames(pageframe_count(sizeof(*pd))))) {
         * err = -ENOMEM;
         * goto err_pmem_alloc_frames;
         * }
         * 
         * pd = pageframe_address(pfindex);
         */

        err = pmem_ref_frames(pageframe_index(&kernel_pd),
                              pageframe_count(sizeof(*pd)));
        if (err)
        {
                goto err_pmem_ref_page_frames;
        }

        pd = &kernel_pd;

        if ((err = page_directory_init(pd)) < 0)
        {
                goto err_page_directory_init;
        }

        /*
         * init address space for kernel task 
         */

        if ((err = vmem_init(as, PAGING_32BIT, pd)) < 0)
        {
                goto err_vmem_init;
        }

        err = 0;

        /*
         * install page tables in all kernel areas 
         */

        for (name = 0; (name < LAST_VMEM_AREA) && !(err < 0); ++name)
        {
                const struct vmem_area *area;
                unsigned long ptindex, ptcount;

                area = vmem_area_get_by_name(name);

                if (!(area->flags & VMEM_AREA_FLAG_PAGETABLES))
                {
                        continue;
                }

                ptindex = pagetable_index(page_address(area->pgindex));
                ptcount = pagetable_count(page_address(area->pgindex),
                                          page_memory(area->npages));

                /*
                 * create page tables for low area 
                 */

                err = vmem_alloc_page_tables_nopg(as, ptindex, ptcount,
                                                  PDE_FLAG_PRESENT|
                                                  PDE_FLAG_WRITEABLE);
        }

        if (err < 0)
        {
                goto err_vmem_alloc_page_tables_nopg;
        }

        /*
         * create identity mapping for all identity areas 
         */

        for (name = 0; (name < LAST_VMEM_AREA) && !(err < 0); ++name)
        {
                const struct vmem_area *area;

                area = vmem_area_get_by_name(name);

                if (!(area->flags & VMEM_AREA_FLAG_POLUTE))
                {
                        continue;
                }

                if (area->flags & VMEM_AREA_FLAG_IDENTITY)
                {
                        err = vmem_map_pageframes_nopg(as,
                                                       area->pgindex,
                                                       area->pgindex,
                                                       area->npages,
                                                       PTE_FLAG_PRESENT|
                                                       PTE_FLAG_WRITEABLE);
                }
                else
                {
                        os_index_t pfindex =
                                pmem_alloc_frames(
                                        pageframe_count(
                                                page_memory(area->npages)));
                        if (!pfindex)
                        {
                                err = -1;
                                break;
                        }

                        err = vmem_map_pageframes_nopg(as, pfindex,
                                                       area->pgindex,
                                                       area->npages,
                                                       PTE_FLAG_PRESENT|
                                                       PTE_FLAG_WRITEABLE);
                }
        }

        if (err < 0)
        {
                goto err_vmem_map_pageframes_nopg;
        }

        /*
         * prepare temporary mappings 
         */

        if ((err = vmem_install_tmp_nopg(as)) < 0)
        {
                goto err_vmem_install_tmp_nopg;
        }

        return 0;

err_vmem_install_tmp_nopg:
err_vmem_map_pageframes_nopg:
err_vmem_alloc_page_tables_nopg:
        vmem_uninit(as);
err_vmem_init:
        page_directory_uninit(pd);
err_page_directory_init:
        pmem_unref_frames(pageframe_index(pd), pageframe_count(sizeof(*pd)));
err_pmem_ref_page_frames:
        return err;
}

static int
vmem_helper_flat_copy_areas(const struct vmem *src_as,
                                     struct vmem *dst_as,
                                     unsigned long pteflags)
{
        enum vmem_area_name name;

        for (name = 0; name < LAST_VMEM_AREA; ++name)
        {
                const struct vmem_area *area = vmem_area_get_by_name(name);

                if (area->flags & pteflags)
                {
                        __vmem_share_2nd_lvl_ps(dst_as, src_as,
                                                area->pgindex, area->npages);
                }
        }

        return 0;
}

int
vmem_helper_init_vmem_from_parent(struct vmem *parent, struct vmem *as)
{
        int err;
        os_index_t pgindex;
        struct page_directory *pd;

        /*
         * create page directory (has to be at page boundary) 
         */

        pgindex = vmem_helper_alloc_pages_in_area(parent,
                                                  VMEM_AREA_KERNEL,
                                                  page_count(0, sizeof(*pd)),
                                                  PTE_FLAG_PRESENT|
                                                  PTE_FLAG_WRITEABLE);
        if (pgindex < 0)
        {
                err = pgindex;
                goto err_vmem_helper_alloc_pages_in_area;
        }

        pd = page_address(pgindex);

        if ((err = page_directory_init(pd)) < 0)
        {
                goto err_page_directory_init;
        }

        /*
         * init address space 
         */

        if ((err = vmem_init(as, PAGING_32BIT, pd)) < 0)
        {
                goto err_vmem_init;
        }

        /*
         * flat-copy page directory from parent 
         */

        err = vmem_helper_flat_copy_areas(parent, as, VMEM_AREA_FLAG_GLOBAL);

        if (err < 0)
        {
                goto err_vmem_helper_flat_copy_areas;
        }

        return 0;

err_vmem_helper_flat_copy_areas:
        vmem_uninit(as);
err_vmem_init:
        page_directory_uninit(pd);
err_page_directory_init:
        /*
         * TODO: unmap page-directory pages 
         */
err_vmem_helper_alloc_pages_in_area:
        return err;
}

int
vmem_helper_allocate_vmem_from_parent(struct vmem *parent, struct vmem **as)
{
        int err;

        if (!(*as = kmalloc(sizeof(**as))))
        {
                err = -ENOMEM;
                goto err_kmalloc_as;
        }

        err = vmem_helper_init_vmem_from_parent(parent, *as);

        if (err < 0)
        {
                goto err_vmem_helper_init_vmem_from_parent;
        }

        return 0;

err_vmem_helper_init_vmem_from_parent:
        kfree(*as);
err_kmalloc_as:
        return err;
}

os_index_t
vmem_helper_alloc_pages_in_area(struct vmem * as,
                            enum vmem_area_name areaname,
                            size_t npages, unsigned int pteflags)
{
        const struct vmem_area *area;

        area = vmem_area_get_by_name(areaname);

        return vmem_alloc_pages_within(as, area->pgindex,
                                           area->pgindex+area->npages, npages,
                                           pteflags);
}

os_index_t
vmem_helper_map_pages_in_area(struct vmem * dst_as,
                          enum vmem_area_name dst_areaname,
                          struct vmem * src_as,
                          os_index_t src_pgindex,
                          size_t pgcount, unsigned long dst_pteflags)
{
        const struct vmem_area *dst_area;

        dst_area = vmem_area_get_by_name(dst_areaname);

        return vmem_map_pages_within(dst_as, dst_area->pgindex,
                                     dst_area->pgindex+dst_area->npages,
                                     src_as, src_pgindex, pgcount,
                                     dst_pteflags);
}

os_index_t
vmem_helper_empty_pages_in_area(struct vmem *vmem,
                                enum vmem_area_name areaname, size_t pgcount)
{
        const struct vmem_area *area;

        area = vmem_area_get_by_name(areaname);

        return vmem_empty_pages_within(vmem, area->pgindex,
                                             area->pgindex+area->npages,
                                             pgcount);
}
