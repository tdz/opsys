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

#include <sys/types.h>

#include <spinlock.h>

/* physical memory */
#include "pageframe.h"
#include "physmem.h"

/* virtual memory */
#include "page.h"
#include "pte.h"
#include "pagetbl.h"
#include "pde.h"
#include "pagedir.h"
#include "vmemarea.h"

#include "addrspace.h"
#include "addrspacehlp.h"

int
address_space_helper_init_kernel_address_space(struct page_directory *kernel_pd,
                                               struct address_space *as)
{
        int err;
        enum virtmem_area_name name;

        /* init page directory and address space for kernel task */

        if ((err = page_directory_init(kernel_pd)) < 0) {
                goto err_page_directory_init;
        }

        if ((err = address_space_init(as, PAGING_32BIT, kernel_pd)) < 0) {
                goto err_address_space_init;
        }

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

                err = address_space_alloc_page_tables_nopg(as,
                                                           ptindex,
                                                           ptcount,
                                                           PDE_FLAG_PRESENT|
                                                           PDE_FLAG_WRITEABLE);
        }

        if (err < 0) {
                goto err_address_space_alloc_page_tables_nopg;
        }

        /* create identity mapping for all identity areas */

        for (name = 0; (name < LAST_VIRTMEM_AREA) && !(err < 0); ++name) {

                const struct virtmem_area *area;

                area = virtmem_area_get_by_name(name);

                if (!(area->flags&VIRTMEM_AREA_FLAG_POLUTE)) {
                        continue;
                }

                if (area->flags&VIRTMEM_AREA_FLAG_IDENTITY) {
                        err = address_space_map_pageframes_nopg(as,
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

                        err = address_space_map_pageframes_nopg(as,
                                                                pfindex,
                                                                area->pgindex,
                                                                area->npages,
                                                                PTE_FLAG_PRESENT|
                                                                PTE_FLAG_WRITEABLE);
                }
        }

        if (err < 0) {
                goto err_address_space_map_pageframes_nopg;
        }

        /* prepare temporary mappings */

        if ((err = address_space_install_tmp(as)) < 0) {
                goto err_address_space_install_tmp;
        }

        return 0;

err_address_space_install_tmp:
err_address_space_map_pageframes_nopg:
err_address_space_alloc_page_tables_nopg:
err_address_space_init:
err_page_directory_init:
        return err;

}

