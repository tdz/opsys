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
#include <types.h>

#include "bitset.h"

#include "page.h"
#include "pte.h"
#include "pagetbl.h"
#include "pde.h"
#include "pagedir.h"
#include "virtmem.h"
#include "task.h"

static unsigned char g_taskid[1024>>3];

int
task_init(struct task *task, struct page_directory *pd)
{
        int err;
        ssize_t taskid;

        if ((taskid = bitset_find_unset(g_taskid, sizeof(g_taskid))) < 0) {
                err = taskid;
                goto err_find_taskid;
        }

        bitset_set(g_taskid, taskid);

        task->pd = pd;
        task->nthreads = 0;
        task->id = taskid;

        return 0;

err_find_taskid:
        return err;
}

int
task_init_from_parent(struct task *tsk, const struct task *parent)
{
        int err;
        struct page_directory *pd;

        /* flat-copy page directory from parent */

        pd = virtmem_alloc_in_area(parent->pd,
                                   page_count(0, sizeof(*pd)),
                                   g_virtmem_area+VIRTMEM_AREA_KERNEL,
                                   PTE_FLAG_PRESENT|PTE_FLAG_WRITEABLE);
        if (!pd) {
                err = -ENOMEM;
                goto err_virtmem_alloc_in_area;
        }

        if ((err = page_directory_init(pd)) < 0) {
                goto err_page_directory_init;
        }

        if ((err = virtmem_flat_copy_areas(parent->pd,
                                           pd,
                                           VIRTMEM_AREA_FLAG_GLOBAL)) < 0) {
                goto err_virtmem_flat_copy_areas;
        }

        /* init task */

        if ((err = task_init(tsk, pd)) < 0) {
                goto err_task_init;
        }

        return 0;

err_task_init:
        /* TODO: free pages */
err_virtmem_flat_copy_areas:
err_page_directory_init:
err_virtmem_alloc_in_area:
        return err;
}

void
task_uninit(struct task *task)
{
        bitset_unset(g_taskid, task->id);
}

int
task_ref(struct task *task)
{
        if (task->nthreads >= 255) {
                return -EAGAIN;
        }

        return ++task->nthreads;
}

int
task_unref(struct task *task)
{
        if (!task->nthreads) {
                return -EINVAL;
        }

        return --task->nthreads;
}

