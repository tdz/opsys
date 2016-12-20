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

#include "vmem.h"
#include <errno.h>
#include "cpu.h"
#include "vmem_32.h"

int
vmem_map_pageframes_nopg(struct vmem *vmem, os_index_t pfindex,
                         os_index_t pgindex, size_t count, unsigned int flags)
{
        int err = 0;

        while (count && !(err < 0))
        {
                err = vmem_32_map_pageframe_nopg(vmem->tlps, pfindex, pgindex, flags);
                ++pgindex;
                ++pfindex;
                --count;
        }

        return err;
}

int
vmem_alloc_page_tables_nopg(struct vmem *vmem, os_index_t ptindex,
                            size_t ptcount, unsigned int flags)
{
        int err;

        for (err = 0; ptcount && !(err < 0); ++ptindex, --ptcount)
        {
                err = vmem_32_alloc_page_table_nopg(vmem->tlps, ptindex, flags);
        }

        return err;
}

int
vmem_install_tmp_nopg(struct vmem *vmem)
{
        return vmem_32_install_tmp_nopg(vmem->tlps);
}

/*
 * public functions
 */

int
vmem_init(struct vmem *vmem, void *tlps)
{
        int err;

        if ((err = semaphore_init(&vmem->sem, 1)) < 0)
        {
                goto err_semaphore_init;
        }

        vmem->tlps = tlps;

        return 0;

err_semaphore_init:
        return err;
}

void
vmem_uninit(struct vmem *vmem)
{
        semaphore_uninit(&vmem->sem);
}

void
vmem_enable(const struct vmem *vmem)
{
        vmem_32_enable(vmem->tlps);
}

int
vmem_alloc_frames(struct vmem *vmem, os_index_t pfindex, os_index_t pgindex,
                  size_t pgcount, unsigned int pteflags)
{
        int err;

        semaphore_enter(&vmem->sem);

        err = vmem_32_alloc_frames(vmem->tlps, pfindex, pgindex, pgcount, pteflags);

        if (err < 0)
        {
                goto err_vmem_alloc_pageframes;
        }

        semaphore_leave(&vmem->sem);

        return 0;

err_vmem_alloc_pageframes:
        semaphore_leave(&vmem->sem);
        return err;
}

os_index_t
vmem_lookup_frame(struct vmem *vmem, os_index_t pgindex)
{
        os_index_t pfindex;

        semaphore_enter(&vmem->sem);

        pfindex = vmem_32_lookup_frame(vmem->tlps, pgindex);

        if (pfindex < 0)
        {
                goto err_vmem_lookup_pageframe;
        }

        semaphore_leave(&vmem->sem);

        return pfindex;

err_vmem_lookup_pageframe:
        semaphore_leave(&vmem->sem);
        return pfindex;
}

static size_t
check_pages_empty(const struct vmem *vmem, os_index_t pgindex, size_t pgcount)
{
        return vmem_32_check_empty_pages(vmem->tlps, pgindex, pgcount);
}

static os_index_t
find_empty_pages(const struct vmem *vmem, size_t npages,
                 os_index_t pgindex_beg, os_index_t pgindex_end)
{
        /*
         * find continuous area in virtual memory
         */

        while ((pgindex_beg < pgindex_end)
               && (npages < (pgindex_end - pgindex_beg)))
        {
                size_t nempty;

                nempty = check_pages_empty(vmem, pgindex_beg, npages);

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

os_index_t
vmem_alloc_pages_at(struct vmem *vmem, os_index_t pgindex, size_t pgcount,
                    unsigned int pteflags)
{
        int err;

        semaphore_enter(&vmem->sem);

        err = vmem_32_alloc_pages(vmem->tlps, pgindex, pgcount, pteflags);

        if (err < 0)
        {
                goto err_vmem_alloc_pages;
        }

        semaphore_leave(&vmem->sem);

        return 0;

err_vmem_alloc_pages:
        semaphore_leave(&vmem->sem);
        return err;
}

os_index_t
vmem_alloc_pages_within(struct vmem *vmem, os_index_t pgindex_min,
                        os_index_t pgindex_max, size_t npages,
                        unsigned int pteflags)
{
        int err;
        os_index_t pgindex;

        semaphore_enter(&vmem->sem);

        pgindex = find_empty_pages(vmem, npages, pgindex_min, pgindex_max);

        if (pgindex < 0)
        {
                err = pgindex;
                goto err_vmem_find_empty_pages;
        }

        err = vmem_32_alloc_pages(vmem->tlps, pgindex, npages, pteflags);

        if (err < 0)
        {
                goto err_vmem_alloc_pages;
        }

        semaphore_leave(&vmem->sem);

        return pgindex;

err_vmem_alloc_pages:
err_vmem_find_empty_pages:
        semaphore_leave(&vmem->sem);
        return err;
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
vmem_map_pages_at(struct vmem *dst_vmem, os_index_t dst_pgindex,
                  struct vmem *src_vmem, os_index_t src_pgindex,
                  size_t pgcount, unsigned long pteflags)
{
        int err;

        semaphore_enter_ordered(&dst_vmem->sem, &src_vmem->sem);

        err = vmem_32_map_pages(dst_vmem->tlps, dst_pgindex, src_vmem, src_pgindex,
                                pgcount, pteflags);
        if (err < 0)
        {
                goto err_vmem_map_pages;
        }

        semaphore_leave_ordered(&dst_vmem->sem, &src_vmem->sem);

        return 0;

err_vmem_map_pages:
        semaphore_leave_ordered(&dst_vmem->sem, &src_vmem->sem);
        return err;
}

os_index_t
vmem_map_pages_within(struct vmem *dst_vmem, os_index_t pg_index_min,
                      os_index_t pg_index_max, struct vmem *src_vmem,
                      os_index_t src_pgindex, size_t pgcount,
                      unsigned long dst_pteflags)
{
        int err;
        os_index_t dst_pgindex;

        semaphore_enter_ordered(&dst_vmem->sem, &src_vmem->sem);

        dst_pgindex = find_empty_pages(dst_vmem, pgcount, pg_index_min,
                                       pg_index_max);
        if (dst_pgindex < 0)
        {
                err = dst_pgindex;
                goto err_vmem_find_empty_pages;
        }

        err = vmem_32_map_pages(dst_vmem->tlps, dst_pgindex,
                                src_vmem, src_pgindex, pgcount, dst_pteflags);
        if (err < 0)
        {
                goto err_vmem_map_pages;
        }

        semaphore_leave_ordered(&dst_vmem->sem, &src_vmem->sem);

        return dst_pgindex;

err_vmem_map_pages:
err_vmem_find_empty_pages:
        semaphore_leave_ordered(&dst_vmem->sem, &src_vmem->sem);
        return err;
}

os_index_t
vmem_empty_pages_within(struct vmem *vmem, os_index_t pg_index_min,
                        os_index_t pg_index_max, size_t pgcount)
{
        int err;
        os_index_t pgindex;

        semaphore_enter(&vmem->sem);

        pgindex = find_empty_pages(vmem, pgcount, pg_index_min, pg_index_max);

        if (pgindex < 0)
        {
                err = pgindex;
                goto err_vmem_find_empty_pages;
        }

        semaphore_leave(&vmem->sem);

        return pgindex;

err_vmem_find_empty_pages:
        semaphore_leave(&vmem->sem);
        return err;
}

int
vmem_share_2nd_lvl_ps(struct vmem *dst_vmem, const struct vmem *src_vmem,
                      os_index_t pgindex, size_t pgcount)
{
        return vmem_32_share_2nd_lvl_ps(dst_vmem->tlps,
                                        src_vmem->tlps,
                                        pgindex, pgcount);
}

/*
 * fault handlers
 */

#include "console.h"

void
vmem_segfault_handler(void *ip)
{
        console_printf("segmentation fault: ip=%x.\n", (unsigned long)ip);
}

void
vmem_pagefault_handler(void *ip, void *addr, unsigned long errcode)
{
        console_printf("page fault: ip=%x, addr=%x, errcode=%x.\n",
                       (unsigned long)ip,
                       (unsigned long)addr, (unsigned long)errcode);

        while (1)
        {
                hlt();
        }
}

