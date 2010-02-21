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

#include <sys/types.h>

#include <cpu.h>

#include "spinlock.h"

/* virtual memory */
#include "vmemarea.h"
#include <addrspace.h>
#include "virtmem.h"

int
virtmem_alloc_page_frames(struct address_space *as, os_index_t pfindex,
                                                    os_index_t pgindex,
                                                    size_t pgcount,
                                                    unsigned int pteflags)
{
        return address_space_alloc_pageframes(as,
                                              pfindex,
                                              pgindex,
                                              pgcount,
                                              pteflags);
}

os_index_t
virtmem_alloc_pages_at(struct address_space *as, os_index_t pgindex,
                                                 size_t pgcount,
                                                 unsigned int pteflags)
{
        return address_space_alloc_pages(as, pgindex, pgcount, pteflags);
}

os_index_t
virtmem_lookup_pageframe(const struct address_space *as, os_index_t pgindex)
{
        return address_space_lookup_pageframe(as, pgindex);
}

os_index_t
virtmem_alloc_pages_in_area(struct address_space *as,
                            size_t npages,
                            enum virtmem_area_name areaname,
                            unsigned int pteflags)
{
        int err;
        os_index_t pgindex;
        const struct virtmem_area *area;

        area = virtmem_area_get_by_name(areaname);

        pgindex = address_space_find_empty_pages(as, npages, area->pgindex,
                                                             area->pgindex+
                                                             area->npages);
        if (pgindex < 0) {
                err = pgindex;
                goto err_page_directory_find_empty_pages;
        }

        err = address_space_alloc_pages(as, pgindex, npages, pteflags);

        if (err < 0) {
                goto err_page_directory_alloc_pages_at;
        }

        return pgindex;

err_page_directory_alloc_pages_at:
err_page_directory_find_empty_pages:
        return err;
}

int
virtmem_flat_copy_areas(const struct address_space *src_as,
                              struct address_space *dst_as,
                              unsigned long pteflags)
{
        enum virtmem_area_name name;

        for (name = 0; name < LAST_VIRTMEM_AREA; ++name) {

                const struct virtmem_area *area =
                        virtmem_area_get_by_name(name);

                if (area->flags&pteflags) {
                        address_space_share_2nd_lvl_ps(dst_as,
                                                       src_as,
                                                       area->pgindex,
                                                       area->npages);
                }
        }

        return 0;
}

int
virtmem_map_pages_at(const struct address_space *src_as,
                           os_index_t src_pgindex,
                           size_t pgcount,
                           struct address_space *dst_as,
                           os_index_t dst_pgindex,
                           unsigned long dst_pteflags)
{
        return address_space_map_pages(src_as,
                                       src_pgindex, pgcount,
                                       dst_as,
                                       dst_pgindex,
                                       dst_pteflags);
}

os_index_t
virtmem_map_pages_in_area(const struct address_space *src_as,
                                os_index_t src_pgindex,
                                size_t pgcount,
                                struct address_space *dst_as,
                                enum virtmem_area_name dst_areaname,
                                unsigned long dst_pteflags)
{
        int err;
        os_index_t dst_pgindex;
        const struct virtmem_area *dst_area;

        dst_area = virtmem_area_get_by_name(dst_areaname);

        dst_pgindex = address_space_find_empty_pages(dst_as,
                                                     pgcount,
                                                     dst_area->pgindex,
                                                     dst_area->pgindex+
                                                     dst_area->npages);
        if (dst_pgindex < 0) {
                err = dst_pgindex;
                goto err_address_space_find_empty_pages;
        }

        err = address_space_map_pages(src_as,
                                      src_pgindex, pgcount,
                                      dst_as,
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
        console_printf("page fault: ip=%x, addr=%x, errcode=%x.\n",
                        (unsigned long)ip,
                        (unsigned long)addr,
                        (unsigned long)errcode);

        while (1) {
                hlt();
        }
}

