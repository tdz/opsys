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

int
task_helper_allocate_task(struct vmem* kernel_as, struct task** task_out)
{
    struct task* task = kmalloc(sizeof(*task));
    if (!task) {
        return -ENOMEM;
    }

    int res = task_init(task, kernel_as);
    if (res < 0) {
        goto err_task_init;
    }

    *task_out = task;

    return 0;

err_task_init:
    kfree(task);
    return res;
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
allocate_vmem_from_parent(struct vmem* parent, struct vmem** vmem_out)
{
    struct vmem* vmem = kmalloc(sizeof(*vmem));
    if (!vmem) {
        return -ENOMEM;
    }

    int res = vmem_init_from_parent(vmem, parent);
    if (res < 0) {
        goto err_vmem_init_from_parent;
    }

    *vmem_out = vmem;

    return 0;

err_vmem_init_from_parent:
    kfree(vmem);
    return res;
}

int
task_helper_init_task_from_parent(const struct task *parent, struct task *tsk)
{
        int err;
        struct vmem *as;

        /*
         * create address space from parent
         */

        err = allocate_vmem_from_parent(parent->as, &as);
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
