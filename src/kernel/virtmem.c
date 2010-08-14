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
#include "semaphore.h"

/* virtual memory */
#include "vmemarea.h"
#include <vmem.h>
#include "virtmem.h"

os_index_t
virtmem_alloc_pages_in_area(struct vmem * as,
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
virtmem_map_pages_in_area(struct vmem * dst_as,
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
                       (unsigned long)addr, (unsigned long)errcode);

        while (1)
        {
                hlt();
        }
}
