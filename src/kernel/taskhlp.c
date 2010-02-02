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

#include <mmu.h>

/* physical memory */
#include <pageframe.h>
#include "physmem.h"

/* virtual memory */
#include <page.h>
#include <pte.h>
#include <pde.h>
#include <pagedir.h>
#include "vmemarea.h"
#include "virtmem.h"

#include "task.h"
#include "taskhlp.h"

int
task_helper_allocate_kernel_task(struct page_directory *kernel_pd,
                                 struct task **tsk)
{
        int err;
        os_index_t pgindex;

        /* init page directory for kernel task */

        if ((err = page_directory_init(kernel_pd)) < 0) {
                goto err_page_directory_init;
        }

        /* build kernel area */

        if ((err = virtmem_init(kernel_pd)) < 0) {
                goto err_virtmem_install;
        }

        /* enable paging */

        mmu_load(((unsigned long)kernel_pd->entry)&(~0xfff));
        mmu_enable_paging();

        /* create kernel task */

        pgindex = virtmem_alloc_pages_in_area(kernel_pd,
                                              page_count(0, sizeof(**tsk)),
                                              VIRTMEM_AREA_KERNEL,
                                              PTE_FLAG_PRESENT|
                                              PTE_FLAG_WRITEABLE);
        if (pgindex < 0) {
                err = pgindex;
                goto err_virtmem_alloc_pages_in_area_tsk;
        }

        *tsk = page_address(pgindex);

        if ((err = task_init(*tsk, kernel_pd)) < 0) {
                goto err_task_init;
        }

        return 0;

err_task_init:
        physmem_unref_frames(pageframe_index(*tsk),
                             pageframe_count(sizeof(**tsk)));
err_virtmem_alloc_pages_in_area_tsk:
err_virtmem_install:
        page_directory_uninit(kernel_pd);
err_page_directory_init:
        physmem_unref_frames(pageframe_index(kernel_pd),
                             pageframe_count(sizeof(*kernel_pd)));
        return err;
}

int
task_helper_allocate_task_from_parent(const struct task *parent,
                                            struct task **tsk)
{
        int err;
        os_index_t pgindex;

        /* allocate task memory */

        pgindex = virtmem_alloc_pages_in_area(parent->pd,
                                              page_count(0, sizeof(*tsk)),
                                              VIRTMEM_AREA_KERNEL,
                                              PTE_FLAG_PRESENT|
                                              PTE_FLAG_WRITEABLE);
        if (pgindex < 0) {
                err = pgindex;
                goto err_virtmem_alloc_pages_in_area;
        }

        *tsk = page_address(pgindex);

        /* init task from parent */

        if ((err = task_helper_init_task_from_parent(parent, tsk)) < 0) {
                goto err_task_helper_init_task_from_parent;
        }

        return 0;

err_task_helper_init_task_from_parent:
        /* TODO: free task pages */
err_virtmem_alloc_pages_in_area:
        return err;
}

int
task_helper_init_task_from_parent(const struct task *parent,
                                        struct task **tsk)
{
        int err;
        os_index_t pgindex;
        struct page_directory *pd;

        /* flat-copy page directory from parent */

        pgindex = virtmem_alloc_pages_in_area(parent->pd,
                                              page_count(0, sizeof(*pd)),
                                              VIRTMEM_AREA_KERNEL,
                                              PTE_FLAG_PRESENT|
                                              PTE_FLAG_WRITEABLE);
        if (pgindex < 0) {
                err = pgindex;
                goto err_virtmem_alloc_pages_in_area;
        }

        pd = page_address(pgindex);

        if ((err = page_directory_init(pd)) < 0) {
                goto err_page_directory_init;
        }

        if ((err = virtmem_flat_copy_areas(parent->pd,
                                           pd,
                                           VIRTMEM_AREA_FLAG_GLOBAL)) < 0) {
                goto err_virtmem_flat_copy_areas;
        }

        /* init task */

        if ((err = task_init(*tsk, pd)) < 0) {
                goto err_task_init;
        }

        return 0;

err_task_init:
        /* TODO: free pages */
err_virtmem_flat_copy_areas:
err_page_directory_init:
err_virtmem_alloc_pages_in_area:
        return err;
}

