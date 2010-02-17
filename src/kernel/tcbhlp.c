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

/* virtual memory */
#include <page.h>
#include <pte.h>
#include "vmemarea.h"
#include "virtmem.h"

#include "spinlock.h"
#include "list.h"
#include "ipcmsg.h"

#include "task.h"
#include <tcb.h>
#include "tcbhlp.h"

int
tcb_helper_allocate_tcb(struct task *tsk, void *stack, struct tcb **tcb)
{
        int err;
        long pgindex;

        pgindex = virtmem_alloc_pages_in_area(tsk->pd,
                                              page_count(0, sizeof(*tcb)),
                                              VIRTMEM_AREA_KERNEL,
                                              PTE_FLAG_PRESENT|
                                              PTE_FLAG_WRITEABLE);
        if (pgindex < 0) {
                err = pgindex;
                goto err_virtmem_alloc_pages_in_area;
        }

        *tcb = page_address(pgindex);

        if ((err = tcb_init(*tcb, tsk, stack)) < 0) {
                goto err_tcb_init;
        }

        return 0;

err_tcb_init:
        /* TODO: free pages */
err_virtmem_alloc_pages_in_area:
        return err;
}

int
tcb_helper_allocate_tcb_and_stack(struct task *tsk, size_t stackpages,
                                  struct tcb **tcb)
{
        os_index_t pgindex;
        int err;
        void *stack;

        pgindex = virtmem_alloc_pages_in_area(tsk->pd,
                                              stackpages,
                                              VIRTMEM_AREA_USER,
                                              PTE_FLAG_PRESENT|
                                              PTE_FLAG_WRITEABLE);
        if (pgindex < 0) {
                err = pgindex;
                goto err_virtmem_alloc_pages_in_area;
        }

        stack = page_address(pgindex+stackpages);

        if ((err = tcb_helper_allocate_tcb(tsk, stack, tcb)) < 0) {
                goto err_tcb_helper_allocate_tcb;
        }

        return 0;

err_tcb_helper_allocate_tcb:
        /* TODO: free stack */
err_virtmem_alloc_pages_in_area:
        return err;
}

int
tcb_helper_run_kernel_thread(struct tcb *tcb, void (*func)(struct tcb*))
{
        tcb_set_initial_ready_state(tcb,
                                    (void*)func,
                                    0,
                                    tcb->stack,
                                    1,
                                    (unsigned long)tcb);

        tcb_set_state(tcb, THREAD_STATE_READY);

        return 0;
}

int
tcb_helper_run_user_thread(struct tcb *cur_tcb, struct tcb *usr_tcb, void *ip)
{
        int err;
        os_index_t stackpage;

        stackpage = virtmem_map_pages_in_area(usr_tcb->task->pd,
                                              page_index(usr_tcb->stack-PAGE_SIZE),
                                              1,
                                              cur_tcb->task->pd,
                                              VIRTMEM_AREA_KERNEL,
                                              PTE_FLAG_PRESENT|
                                              PTE_FLAG_WRITEABLE);
        if (stackpage < 0) {
                err = stackpage;
                goto err_virtmem_map_pages_in_area;
        }

        /* set thread to starting state */
        tcb_set_initial_ready_state(usr_tcb,
                                    ip,
                                    0,
                                    page_address(stackpage)+PAGE_SIZE,
                                    0);

        tcb_set_state(usr_tcb, THREAD_STATE_READY);

        /* TODO: unmap stack from current address space */

        return 0;

err_virtmem_map_pages_in_area:
        return err;
}

