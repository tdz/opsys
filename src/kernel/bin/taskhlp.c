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

#include "taskhlp.h"
#include <errno.h>
#include "alloc.h"
#include "page.h"
#include "pte.h"
#include "task.h"
#include "vmem.h"
#include "vmemhlp.h"

int
task_helper_init_kernel_task(struct vmem *kernel_as, struct task **tsk)
{
        int err;
        os_index_t pgindex;

        /*
         * create kernel task
         */

        pgindex = vmem_helper_alloc_pages_in_area(kernel_as,
                                                  VMEM_AREA_KERNEL,
                                                  page_count(0, sizeof(**tsk)),
                                                  PTE_FLAG_PRESENT|
                                                  PTE_FLAG_WRITEABLE);
        if (pgindex < 0)
        {
                err = pgindex;
                goto err_vmem_helper_alloc_pages_in_area_tsk;
        }

        *tsk = page_address(pgindex);

        if ((err = task_init(*tsk, kernel_as)) < 0)
        {
                goto err_task_init;
        }

        return 0;

err_task_init:
/*        pmem_unref_frames(pageframe_index(*tsk),
                             pageframe_count(sizeof(**tsk)));*/
err_vmem_helper_alloc_pages_in_area_tsk:
        return err;
}

int
task_helper_allocate_task_from_parent(const struct task *parent,
                                      struct task **tsk)
{
        int err;

        /*
         * allocate task memory
         */

        if (!(*tsk = kmalloc(sizeof(**tsk))))
        {
                err = -ENOMEM;
                goto err_kmalloc_tsk;
        }

        /*
         * init task from parent
         */

        if ((err = task_helper_init_task_from_parent(parent, *tsk)) < 0)
        {
                goto err_task_helper_init_task_from_parent;
        }

        return 0;

err_task_helper_init_task_from_parent:
        kfree(*tsk);
err_kmalloc_tsk:
        return err;
}

#include "console.h"

int
task_helper_init_task_from_parent(const struct task *parent, struct task *tsk)
{
        int err;
        struct vmem *as;

        /*
         * create address space from parent
         */

        err = vmem_helper_allocate_vmem_from_parent
                (parent->as, &as);
        if (err < 0)
        {
                goto err_vmem_helper_allocate_vmem_from_parent;
        }

        /*
         * init task
         */

        if ((err = task_init(tsk, as)) < 0)
        {
                goto err_task_init;
        }

        return 0;

err_task_init:
        /*
         * TODO: free address space
         */
err_vmem_helper_allocate_vmem_from_parent:
        return err;
}
