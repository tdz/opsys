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

int
virtmem_alloc_frames(struct vmem *as, os_index_t pfindex,
                     os_index_t pgindex, size_t pgcount,
                     unsigned int pteflags)
{
        int err;

        semaphore_enter(&as->sem);

        err = vmem_alloc_frames(as, pfindex, pgindex, pgcount, pteflags);

        if (err < 0)
        {
                goto err_vmem_alloc_pageframes;
        }

        semaphore_leave(&as->sem);

        return 0;

err_vmem_alloc_pageframes:
        semaphore_leave(&as->sem);
        return err;
}

os_index_t
virtmem_lookup_frame(struct vmem * as, os_index_t pgindex)
{
        os_index_t pfindex;

        semaphore_enter(&as->sem);

        pfindex = vmem_lookup_frame(as, pgindex);

        if (pfindex < 0)
        {
                goto err_vmem_lookup_pageframe;
        }

        semaphore_leave(&as->sem);

        return pfindex;

err_vmem_lookup_pageframe:
        semaphore_leave(&as->sem);
        return pfindex;
}

os_index_t
virtmem_alloc_pages_at(struct vmem * as, os_index_t pgindex,
                       size_t pgcount, unsigned int pteflags)
{
        int err;

        semaphore_enter(&as->sem);

        err = vmem_alloc_pages(as, pgindex, pgcount, pteflags);

        if (err < 0)
        {
                goto err_vmem_alloc_pages;
        }

        semaphore_leave(&as->sem);

        return 0;

err_vmem_alloc_pages:
        semaphore_leave(&as->sem);
        return err;
}

os_index_t
virtmem_alloc_pages_within(struct vmem *as, os_index_t pgindex_min,
                           os_index_t pgindex_max, size_t npages,
                           unsigned int pteflags)
{
        int err;
        os_index_t pgindex;

        semaphore_enter(&as->sem);

        pgindex = vmem_find_empty_pages(as, npages, pgindex_min, pgindex_max);

        if (pgindex < 0)
        {
                err = pgindex;
                goto err_vmem_find_empty_pages;
        }

        err = vmem_alloc_pages(as, pgindex, npages, pteflags);

        if (err < 0)
        {
                goto err_vmem_alloc_pages;
        }

        semaphore_leave(&as->sem);

        return pgindex;

err_vmem_alloc_pages:
err_vmem_find_empty_pages:
        semaphore_leave(&as->sem);
        return err;
}

os_index_t
virtmem_alloc_pages_in_area(struct vmem * as,
                            enum virtmem_area_name areaname,
                            size_t npages, unsigned int pteflags)
{
        const struct virtmem_area *area;

        area = virtmem_area_get_by_name(areaname);

        return virtmem_alloc_pages_within(as, area->pgindex,
                                          area->pgindex+area->npages, npages,
                                          pteflags);
}

static void
semaphore_enter_ordered(struct semaphore *sem1, struct semaphore *sem2)
{
        if (sem1 < sem2)
        {
                semaphore_enter(sem1);
                semaphore_enter(sem2);
        }
        else if (sem1 > sem2)
        {
                semaphore_enter(sem2);
                semaphore_enter(sem1);
        }
        else                    /* sem1 == sem2 */
        {
                semaphore_enter(sem1);
        }
}

static void
semaphore_leave_ordered(struct semaphore *sem1, struct semaphore *sem2)
{
        if (sem1 != sem2)
        {
                semaphore_leave(sem1);
                semaphore_leave(sem2);
        }
        else                    /* sem1 == sem2 */
        {
                semaphore_leave(sem1);
        }
}

int
virtmem_map_pages_at(struct vmem *dst_as, os_index_t dst_pgindex,
                     struct vmem *src_as, os_index_t src_pgindex,
                     size_t pgcount, unsigned long pteflags)
{
        int err;

        semaphore_enter_ordered(&dst_as->sem, &src_as->sem);

        err = vmem_map_pages(dst_as, dst_pgindex,
                             src_as, src_pgindex, pgcount, pteflags);
        if (err < 0)
        {
                goto err_vmem_map_pages;
        }

        semaphore_leave_ordered(&dst_as->sem, &src_as->sem);

        return 0;

err_vmem_map_pages:
        semaphore_leave_ordered(&dst_as->sem, &src_as->sem);
        return err;
}

os_index_t
virtmem_map_pages_within(struct vmem *dst_as, os_index_t pg_index_min,
                         os_index_t pg_index_max, struct vmem *src_as,
                         os_index_t src_pgindex, size_t pgcount,
                         unsigned long dst_pteflags)
{
        int err;
        os_index_t dst_pgindex;

        semaphore_enter_ordered(&dst_as->sem, &src_as->sem);

        dst_pgindex = vmem_find_empty_pages(dst_as, pgcount, pg_index_min,
                                            pg_index_max);
        if (dst_pgindex < 0)
        {
                err = dst_pgindex;
                goto err_vmem_find_empty_pages;
        }

        err = vmem_map_pages(dst_as, dst_pgindex,
                             src_as, src_pgindex, pgcount, dst_pteflags);
        if (err < 0)
        {
                goto err_vmem_map_pages;
        }

        semaphore_leave_ordered(&dst_as->sem, &src_as->sem);

        return dst_pgindex;

err_vmem_map_pages:
err_vmem_find_empty_pages:
        semaphore_leave_ordered(&dst_as->sem, &src_as->sem);
        return err;
}

os_index_t
virtmem_map_pages_in_area(struct vmem * dst_as,
                          enum virtmem_area_name dst_areaname,
                          struct vmem * src_as,
                          os_index_t src_pgindex,
                          size_t pgcount, unsigned long dst_pteflags)
{
        const struct virtmem_area *dst_area;

        dst_area = virtmem_area_get_by_name(dst_areaname);

        return virtmem_map_pages_within(dst_as, dst_area->pgindex,
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
