/*
 *  opsys - A small, experimental operating system
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

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#include "minmax.h"
#include <membar.h>
#include <mmu.h>
#include <cpu.h>

/* physical memory */
#include <pageframe.h>
#include "physmem.h"

/* virtual memory */
#include <page.h>
#include <pte.h>
#include <pagetbl.h>
#include <pde.h>
#include <pagedir.h>
#include "vmemarea.h"
#include "addrspace.h"
#include "virtmem.h"

static int
virtmem_map_page_frame_at_nopg(struct page_directory *pd,
                               os_index_t pfindex,
                               os_index_t pgindex,
                               unsigned int flags)
{
        os_index_t ptindex;
        int err;
        struct page_table *pt;

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
virtmem_map_page_frames_at_nopg(struct page_directory *pd,
                                os_index_t pfindex,
                                os_index_t pgindex,
                                size_t count,
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
virtmem_alloc_page_table_nopg(struct page_directory *pd,
                              os_index_t ptindex,
                              unsigned int flags)
{
        os_index_t pfindex;
        int err;

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

        return page_directory_install_page_table(pd, pfindex, ptindex, flags);

err_page_table_init:
        physmem_unref_frames(pfindex, pageframe_count(sizeof(struct page_table)));
err_physmem_alloc_frames:
        return err;
}

static int
virtmem_alloc_page_tables_nopg(struct page_directory *pd,
                               os_index_t ptindex,
                               size_t ptcount,
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
        const struct virtmem_area *low, *tmp;
        
        low = virtmem_area_get_by_name(VIRTMEM_AREA_LOW);
        tmp = virtmem_area_get_by_name(VIRTMEM_AREA_KERNEL_TMP);

        return page_address(low->pgindex + low->npages - (tmp->npages>>10));
}

int
virtmem_init(struct page_directory *pd)
{
        int err;
        enum virtmem_area_name name;

        err = 0;

        /* install page tables in all kernel areas */

        for (name = 0; (name < LAST_VIRTMEM_AREA) && !(err < 0); ++name) {

                const struct virtmem_area *area;
                unsigned long ptindex, ptcount;

                area = virtmem_area_get_by_name(name);

                if (!(area->flags&VIRTMEM_AREA_FLAG_PAGETABLES)) {
                        continue;
                }

                ptindex = pagetable_index(page_address(area->pgindex));
                ptcount = pagetable_count(page_address(area->pgindex),
                                          page_memory(area->npages));

                /* create page tables for low area */

                err = virtmem_alloc_page_tables_nopg(pd,
                                                     ptindex,
                                                     ptcount,
                                                     PDE_FLAG_PRESENT|
                                                     PDE_FLAG_WRITEABLE);
        }

        /* create identity mapping for all identity areas */

        for (name = 0; (name < LAST_VIRTMEM_AREA) && !(err < 0); ++name) {

                const struct virtmem_area *area;

                area = virtmem_area_get_by_name(name);

                if (!(area->flags&VIRTMEM_AREA_FLAG_POLUTE)) {
                        continue;
                }

                if (area->flags&VIRTMEM_AREA_FLAG_IDENTITY) {
                        err = virtmem_map_page_frames_at_nopg(pd,
                                                              area->pgindex,
                                                              area->pgindex,
                                                              area->npages,
                                                              PTE_FLAG_PRESENT|
                                                              PTE_FLAG_WRITEABLE);
                } else {
                        os_index_t pfindex = physmem_alloc_frames(
                                pageframe_count(page_memory(area->npages)));

                        if (!pfindex) {
                                err = -1;
                                break;
                        }

                        err = virtmem_map_page_frames_at_nopg(pd,
                                                              pfindex,
                                                              area->pgindex,
                                                              area->npages,
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
                os_index_t ptpfindex, index;
                const struct virtmem_area *tmp;

                pt = virtmem_get_page_table_tmp();

                if ((err = page_table_init(pt)) < 0) {
                        return err;
                }

                ptpfindex = page_index(pt);

                tmp = virtmem_area_get_by_name(VIRTMEM_AREA_KERNEL_TMP);

                index = pagetable_index(page_address(tmp->pgindex));

                err = page_directory_install_page_table(pd,
                                ptpfindex,
                                index,
                                PTE_FLAG_PRESENT|PTE_FLAG_WRITEABLE);
        }

        return err;
}

int
virtmem_alloc_page_frames(struct page_directory *pd, os_index_t pfindex,
                                                     os_index_t pgindex,
                                                     size_t pgcount,
                                                     unsigned int pteflags)
{
        return address_space_alloc_page_frames(pd,
                                               pfindex,
                                               pgindex,
                                               pgcount,
                                               pteflags);
}

os_index_t
virtmem_alloc_pages_at(struct page_directory *pd, os_index_t pgindex,
                                                  size_t pgcount,
                                                  unsigned int pteflags)
{
        return address_space_alloc_pages(pd, pgindex, pgcount, pteflags);
}

os_index_t
virtmem_lookup_pageframe(const struct page_directory *pd, os_index_t pgindex)
{
        return address_space_lookup_pageframe(pd, pgindex);
}

os_index_t
virtmem_alloc_pages_in_area(struct page_directory *pd,
                            size_t npages,
                            enum virtmem_area_name areaname,
                            unsigned int pteflags)
{
        int err;
        os_index_t pgindex;
        const struct virtmem_area *area;

        area = virtmem_area_get_by_name(areaname);

        pgindex = address_space_find_empty_pages(pd, npages, area->pgindex,
                                                             area->pgindex+
                                                             area->npages);
        if (pgindex < 0) {
                err = pgindex;
                goto err_page_directory_find_empty_pages;
        }

        err = address_space_alloc_pages(pd, pgindex, npages, pteflags);

        if (err < 0) {
                goto err_page_directory_alloc_pages_at;
        }

        return pgindex;

err_page_directory_alloc_pages_at:
err_page_directory_find_empty_pages:
        return err;
}

int
virtmem_flat_copy_areas(const struct page_directory *pd,
                              struct page_directory *dst,
                              unsigned long flags)
{
        return address_space_flat_copy_areas(pd, dst, flags);
}

int
virtmem_map_pages_at(const struct page_directory *src_pd,
                           os_index_t src_pgindex,
                           size_t pgcount,
                           struct page_directory *dst_pd,
                           os_index_t dst_pgindex,
                           unsigned long dst_pteflags)
{
        return address_space_map_pages(src_pd,
                                       src_pgindex, pgcount,
                                       dst_pd,
                                       dst_pgindex,
                                       dst_pteflags);
}

os_index_t
virtmem_map_pages_in_area(const struct page_directory *src_pd,
                            os_index_t src_pgindex,
                            size_t pgcount,
                            struct page_directory *dst_pd,
                            enum virtmem_area_name dst_areaname,
                            unsigned long dst_pteflags)
{
        int err;
        os_index_t dst_pgindex;
        const struct virtmem_area *dst_area;

        dst_area = virtmem_area_get_by_name(dst_areaname);

        dst_pgindex = address_space_find_empty_pages(dst_pd,
                                                     pgcount,
                                                     dst_area->pgindex,
                                                     dst_area->pgindex+
                                                     dst_area->npages);
        if (dst_pgindex < 0) {
                err = dst_pgindex;
                goto err_address_space_find_empty_pages;
        }

        err = address_space_map_pages(src_pd,
                                      src_pgindex, pgcount,
                                      dst_pd,
                                      dst_pgindex,
                                      dst_pteflags);
        if (err < 0) {
                goto err_address_space_map_pages;
        }

        return dst_pgindex;

err_address_space_map_pages:
err_address_space_find_empty_pages:
        return err;
}

#include "console.h"

void
virtmem_segfault_handler(void *ip)
{
        console_printf("segmentation fault: ip=%x.\n", (unsigned long)ip);
}

void
virtmem_pagefault_handler(void *ip, void *addr, unsigned long errcode)
{
        const struct virtmem_area *area;

        console_printf("page fault: ip=%x, addr=%x, errcode=%x.\n",
                        (unsigned long)ip,
                        (unsigned long)addr,
                        (unsigned long)errcode);

        while (1) {
                hlt();
        }
        area = virtmem_area_get_by_page(page_index(addr));

        if (area->flags&VIRTMEM_AREA_FLAG_KERNEL) {
                do {
                        hlt();
                } while (1);
        }
}

