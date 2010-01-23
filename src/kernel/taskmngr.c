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

#include <types.h>
#include <errno.h>
#include <stddef.h>
#include <page.h>
#include <pte.h>
#include <pagetbl.h>
#include <pde.h>
#include <pagedir.h>
#include <tcb.h>
#include "task.h"
#include "pageframe.h"
#include "physmem.h"
#include "virtmem.h"
#include "tid.h"
#include "taskmngr.h"
#include "console.h"
#include <mmu.h>
#include <string.h>

static struct task*  g_task[1024];
static threadid_type g_current_tid = 0;

int
taskmngr_init(void *stack)
{
        int err;
        int pgindex;
        struct tcb *tcb;

        /* init page directory for kernel task */

        static struct page_directory g_kernel_pd;

        if ((err = page_directory_init(&g_kernel_pd)) < 0) {
                goto err_page_directory_init;
        }

        /* build kernel area */

        if ((err = virtmem_install(&g_kernel_pd)) < 0) {
                goto err_virtmem_install;
        }

        /* enable paging */

        console_printf("enabling paging\n");
        mmu_load(((unsigned long)g_kernel_pd.entry)&(~0xfff));
        mmu_enable_paging();
        console_printf("paging enabled.\n");

        /* create kernel task */

        pgindex = virtmem_alloc_pages_in_area(&g_kernel_pd,
                                              page_count(0, sizeof(*g_task[0])),
                                              g_virtmem_area+
                                                VIRTMEM_AREA_KERNEL,
                                              PTE_FLAG_PRESENT|
                                              PTE_FLAG_WRITEABLE);
        if (pgindex < 0) {
                err = pgindex;
                goto err_virtmem_alloc_pages_in_area_tsk;
        }

        g_task[0] = page_address(pgindex);

        if ((err = task_init(g_task[0], &g_kernel_pd)) < 0) {
                goto err_task_init;
        }

        return 0;

        /* create thread (0:0)
         */

        tcb = task_get_tcb(g_task[0], 0);

        if (!tcb) {
                err = -ENOMEM;
                goto err_task_get_thread;
        }

        if ((err = tcb_set_page_directory(tcb, &g_kernel_pd)) < 0) {
                goto err_tcb_set_page_directory;
        }

        /* save registers in TCB 0 */

        return 0;

err_tcb_set_page_directory:
err_task_get_thread:
        task_uninit(g_task[0]);
err_task_init:
        physmem_unref_frames(pageframe_index((unsigned long)g_task[0]),
                             pageframe_count(sizeof(*g_task[0])));
err_virtmem_alloc_pages_in_area_tsk:
err_virtmem_install:
        page_directory_uninit(&g_kernel_pd);
err_page_directory_init:
        physmem_unref_frames(pageframe_index((unsigned long)&g_kernel_pd),
                             pageframe_count(sizeof(g_kernel_pd)));
        return err;
}

ssize_t
taskmngr_add_task(struct task *tsk)
{
        ssize_t i = 1;

        while ((i < sizeof(g_task)/sizeof(g_task[0])) && g_task[i]) {
                ++i;
        }

        if (i == sizeof(g_task)/sizeof(g_task[0])) {
                return -EAGAIN;
        }

        g_task[i] = tsk;

        return i;
}

ssize_t
taskmngr_allocate_task(struct task *parent)
{
        int err;
        struct page_directory *pd;
        struct task *tsk;

        /* create page directory */

        pd = virtmem_alloc_in_area(parent->pd,
                                   page_count(0, sizeof(*pd)),
                                   g_virtmem_area+VIRTMEM_AREA_KERNEL,
                                   PTE_FLAG_PRESENT|PTE_FLAG_WRITEABLE);
        if (!pd) {
                err = -ENOMEM;
                goto err_virtmem_alloc_in_area_pd;
        }

        if ((err = page_directory_init(pd)) < 0) {
                goto err_page_directory_init;
        }

        if ((err = virtmem_flat_copy_areas(parent->pd,
                                           pd,
                                           VIRTMEM_AREA_FLAG_GLOBAL)) < 0) {
                goto err_virtmem_flat_copy_areas;
        }

        /* create task */

        tsk = virtmem_alloc_in_area(parent->pd,
                                    page_count(0, sizeof(*tsk)),
                                    g_virtmem_area+VIRTMEM_AREA_TASKSTATE,
                                    PTE_FLAG_PRESENT|PTE_FLAG_WRITEABLE);
        if (!tsk) {
                err = -ENOMEM;
                goto err_virtmem_alloc_in_area_tsk;
        }

        if ((err = task_init(tsk, pd)) < 0) {
                goto err_task_init;
        }

        return taskmngr_add_task(tsk);

err_task_init:
        /* TODO: free pages */
err_virtmem_alloc_in_area_tsk:
err_virtmem_flat_copy_areas:
err_page_directory_init:
err_virtmem_alloc_in_area_pd:
        return err;
}

struct task *
taskmngr_get_current_task()
{
        return taskmngr_get_task(threadid_get_taskid(g_current_tid));
}

struct task *
taskmngr_get_task(unsigned int taskid)
{
        return taskid < sizeof(g_task)/sizeof(g_task[0]) ? g_task[taskid]
                                                         : NULL;
}

struct tcb *
taskmngr_get_current_tcb()
{
        return taskmngr_get_tcb(g_current_tid);
}

struct tcb *
taskmngr_get_tcb(threadid_type tid)
{
        struct task *tsk = taskmngr_get_task(threadid_get_taskid(tid));

        return tsk ? task_get_tcb(tsk, threadid_get_tcbid(tid)) : NULL;
}

struct tcb *
taskmngr_switchto(struct tcb *tcb)
{
        return NULL;
}

