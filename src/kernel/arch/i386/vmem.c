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
#include <pmem.h>

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

static int
__vmem_map_pageframe_nopg(struct vmem *vmem, os_index_t pfindex,
                          os_index_t pgindex, unsigned int flags)
{
        /* INDENT-OFF */
        static int (* const map_pageframe_nopg[])(void *,
                                                  os_index_t,
                                                  os_index_t, unsigned int) =
        {
                vmem_32_map_pageframe_nopg,
                vmem_pae_map_pageframe_nopg
        };
        /* INDENT-ON */

        return map_pageframe_nopg[vmem->pgmode](vmem->tlps, pgindex, pgindex,
                                                flags);
}

int
vmem_map_pageframes_nopg(struct vmem *vmem, os_index_t pfindex,
                         os_index_t pgindex, size_t count, unsigned int flags)
{
        int err = 0;

        while (count && !(err < 0))
        {
                err = __vmem_map_pageframe_nopg(vmem, pfindex, pgindex, flags);
                ++pgindex;
                ++pfindex;
                --count;
        }

        return err;
}

static int
__vmem_alloc_page_table_nopg(struct vmem *vmem, os_index_t ptindex,
                             unsigned int flags)
{
        /* INDENT-OFF */
        static int (*const alloc_page_table_pg[]) (void *,
                                                   os_index_t, unsigned int) =
        {
                vmem_32_alloc_page_table_nopg,
                vmem_pae_alloc_page_table_nopg
        };
        /* INDENT-ON */

        return alloc_page_table_pg[vmem->pgmode](vmem->tlps, ptindex, flags);
}

int
vmem_alloc_page_tables_nopg(struct vmem *vmem, os_index_t ptindex,
                            size_t ptcount, unsigned int flags)
{
        int err;

        for (err = 0; ptcount && !(err < 0); ++ptindex, --ptcount)
        {
                err = __vmem_alloc_page_table_nopg(vmem, ptindex, flags);
        }

        return err;
}

int
vmem_install_tmp_nopg(struct vmem *vmem)
{
        /* INDENT-OFF */
        static int (* const install_tmp_nopg[])(void *) =
        {
                vmem_32_install_tmp_nopg,
                vmem_pae_install_tmp_nopg
        };
        /* INDENT-ON */

        return install_tmp_nopg[vmem->pgmode](vmem->tlps);
}

/*
 * public functions
 */

int
vmem_init(struct vmem *vmem, enum paging_mode pgmode, void *tlps)
{
        int err;

        if ((err = semaphore_init(&vmem->sem, 1)) < 0)
        {
                goto err_semaphore_init;
        }

        vmem->pgmode = pgmode;
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
        static void (* const enable[])(const void *tlps) =
        {
                vmem_32_enable,
                vmem_pae_enable
        };

        enable[vmem->pgmode](vmem->tlps);
}

int
vmem_alloc_frames(struct vmem *vmem, os_index_t pfindex, os_index_t pgindex,
                  size_t pgcount, unsigned int pteflags)
{
        int err;

        semaphore_enter(&vmem->sem);

        err = __vmem_alloc_frames(vmem, pfindex, pgindex, pgcount, pteflags);

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

        pfindex = __vmem_lookup_frame(vmem, pgindex);

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
        static size_t(*const check_empty[])(const void*, os_index_t, size_t) =
        {
                vmem_32_check_empty_pages,
                vmem_pae_check_empty_pages
        };

        return check_empty[vmem->pgmode](vmem->tlps, pgindex, pgcount);
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

        err = __vmem_alloc_pages(vmem, pgindex, pgcount, pteflags);

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

        err = __vmem_alloc_pages(vmem, pgindex, npages, pteflags);

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

        err = __vmem_map_pages(dst_vmem, dst_pgindex, src_vmem, src_pgindex,
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

        err = __vmem_map_pages(dst_vmem, dst_pgindex,
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

/*
 * internal functions
 */

int
__vmem_alloc_frames(struct vmem *vmem, os_index_t pfindex, os_index_t pgindex,
                    size_t pgcount, unsigned int pteflags)
{
        /* INDENT-OFF */
        static int (*const alloc_frames[])(void *, os_index_t, os_index_t,
                                           size_t, unsigned int) =
        {
                vmem_32_alloc_frames,
                vmem_pae_alloc_frames
        };
        /* INDENT-ON */

        return alloc_frames[vmem->pgmode](vmem->tlps, pfindex, pgindex,
                                          pgcount, pteflags);
}

os_index_t
__vmem_lookup_frame(const struct vmem *vmem, os_index_t pgindex)
{
        /* INDENT-OFF */
        static os_index_t (*const lookup_frame[])(const void *, os_index_t) =
        {
                vmem_32_lookup_frame,
                vmem_pae_lookup_frame
        };
        /* INDENT-ON */

        return lookup_frame[vmem->pgmode](vmem->tlps, pgindex);
}

int
__vmem_alloc_pages(struct vmem *vmem, os_index_t pgindex, size_t pgcount,
                   unsigned int pteflags)
{
        /* INDENT-OFF */
        static int (*const alloc_pages[])(void *, os_index_t,
                                          size_t, unsigned int) =
        {
                vmem_32_alloc_pages,
                vmem_pae_alloc_pages
        };
        /* INDENT-ON */

        return alloc_pages[vmem->pgmode](vmem->tlps, pgindex, pgcount, pteflags);
}

int
__vmem_map_pages(struct vmem *dst_vmem, os_index_t dst_pgindex,
                 const struct vmem *src_vmem, os_index_t src_pgindex,
                 size_t pgcount, unsigned long pteflags)
{
        /* INDENT-OFF */
        static int (*const map_pages[]) (void *,
                                         os_index_t,
                                         const struct vmem *,
                                         os_index_t, size_t, unsigned long) =
        {
                vmem_32_map_pages,
                vmem_pae_map_pages
        };
        /* INDENT-ON */

        return map_pages[dst_vmem->pgmode](dst_vmem->tlps, dst_pgindex, 
                                           src_vmem, src_pgindex, pgcount,
                                           pteflags);
}

int
__vmem_share_2nd_lvl_ps(struct vmem *dst_vmem, const struct vmem *src_vmem,
                        os_index_t pgindex, size_t pgcount)
{
        /* INDENT-OFF */
        static int (*const share_2nd_lvl_ps[]) (void *,
                                                const void *, os_index_t,
                                                size_t) =
        {
                vmem_32_share_2nd_lvl_ps,
                vmem_pae_share_2nd_lvl_ps
        };
        /* INDENT-ON */

        return share_2nd_lvl_ps[dst_vmem->pgmode](dst_vmem->tlps,
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

