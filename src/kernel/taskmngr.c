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

static threadid_type current_tid = 0;

static struct task* g_task[1024];

#include "console.h"
#include <mmu.h>
#include <string.h>

int
taskmngr_init()
{
        extern struct page_directory *g_current_pd;

        int err;
        static struct page_directory init_pd;
        struct task *task;
        struct tcb *tcb;

        g_current_pd = &init_pd;

        /* create virtual address space */

        if ((err = page_directory_init(g_current_pd)) < 0) {
                goto err_page_directory_init;
        }

        /* build kernel area */

        if ((err = virtmem_install(g_current_pd)) < 0) {
                goto err_virtmem_install;
        }

        /* enable paging */

        console_printf("enabling paging\n");
        mmu_load(((unsigned long)g_current_pd->entry)&(~0xfff));
        mmu_enable_paging();
        console_printf("paging enabled.\n");

        /* test allocation */
        {
                int *addr = page_address(virtmem_alloc_pages_in_area(
                                        g_current_pd,
                                        2,
                                        g_virtmem_area+VIRTMEM_AREA_USER,
                                        PTE_FLAG_PRESENT|
                                        PTE_FLAG_WRITEABLE));

                if (!addr) {
                        console_printf("alloc error %s %x.\n", __FILE__, __LINE__);
                }

                console_printf("alloced addr=%x.\n", addr);

                memset(addr, 0, 2*PAGE_SIZE);
        }

        {
                int *addr = page_address(virtmem_alloc_pages_in_area(
                                        g_current_pd,
                                        1023,
                                        g_virtmem_area+VIRTMEM_AREA_USER,
                                        PTE_FLAG_PRESENT|
                                        PTE_FLAG_WRITEABLE));

                if (!addr) {
                        console_printf("alloc error %s %x.\n", __FILE__, __LINE__);
                }

                console_printf("alloced addr=%x.\n", addr);

                memset(addr, 0, 1023*PAGE_SIZE);
        }

        return 0;


        /* create task 0
         */

        if ( !(task = task_lookup(0)) ) {
                err = -EAGAIN;
                goto err_task_lookup;
        }

        /* allocate memory for task */
        /* FIXME: do this in virtual memory */
        if (!physmem_alloc_frames_at(pageframe_index((unsigned long)task),
                                     pageframe_count(sizeof(*task)))) {
                err = -ENOMEM;
                goto err_task_alloc;
        }

        if ((err = task_init(task, g_current_pd)) < 0) {
                goto err_task_init;
        }

        task->pd = g_current_pd;

        /* create thread (0:0)
         */

        tcb = task_get_tcb(task, 0);

        if (!tcb) {
                err = -ENOMEM;
                goto err_task_get_thread;
        }

        if ((err = tcb_set_page_directory(tcb, g_current_pd)) < 0) {
                goto err_tcb_set_page_directory;
        }

        /* save registers in TCB 0 */

        return 0;

err_tcb_set_page_directory:
err_task_get_thread:
        task_uninit(task);
err_task_init:
        physmem_unref_frames(pageframe_index((unsigned long)task),
                             pageframe_count(sizeof(*task)));
err_task_alloc:
err_task_lookup:
err_virtmem_install:
        page_directory_uninit(g_current_pd);
err_page_directory_init:
        physmem_unref_frames(pageframe_index((unsigned long)g_current_pd),
                             pageframe_count(sizeof(*g_current_pd)));
        return err;
}

struct tcb *
taskmngr_get_current_tcb()
{
        return taskmngr_get_tcb(current_tid);
}

struct tcb *
taskmngr_get_tcb(threadid_type tid)
{
        struct task *tsk =
                page_address(g_virtmem_area[VIRTMEM_AREA_TASKSTATE].pgindex);

        return tsk[threadid_get_taskid(tid)].tcb + threadid_get_tcbid(tid);
}

struct tcb *
taskmngr_switchto(struct tcb *tcb)
{
        return NULL;
}

int
taskmngr_install_task(struct task *tsk)
{
        int i = 0;

        while ((i < sizeof(g_task)/sizeof(g_task[0])) && g_task[i]) {
                ++i;
        }

        if (i == sizeof(g_task)/sizeof(g_task[0])) {
                return -EAGAIN;
        }

        g_task[i] = tsk;

        return i;
}

int
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

        return taskmngr_install_task(tsk);

err_task_init:
        /* TODO: free pages */
err_virtmem_alloc_in_area_tsk:
err_virtmem_flat_copy_areas:
err_page_directory_init:
err_virtmem_alloc_in_area_pd:
        return err;
}

