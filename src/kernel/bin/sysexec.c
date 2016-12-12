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

#include "sysexec.h"
#include "console.h"
#include "cpu.h"
#include "loader.h"
#include "sched.h"
#include "syssrv.h"
#include "taskhlp.h"
#include "tcb.h"
#include "tcbhlp.h"

/**
 * \brief sets up virtual memory and system services
 * \param[in, out] tsk return the kernel task
 * \param[in] vmem kernel-task virtual address space
 * \param[in] stack initial address of the idle thread's stack
 * \return 0 on success, or a negative error code otherwise
 */
int
schedule_kernel_threads(struct vmem* vmem, void* stack, struct task** task_out)
{
    /* build kernel task with idle thread around current execution
     * context */

    struct task* task;
    int res = task_helper_allocate_task(vmem, &task);
    if (res < 0) {
        console_perror("task_helper_allocate_task", -res);
        return res;
    }

    struct tcb *tcb;
    res = tcb_helper_allocate_tcb(task, stack, &tcb);
    if (res < 0) {
        console_perror("tcb_helper_allocate_tcb", -res);
        goto err_tcb_helper_allocate_tcb;
    }

    tcb_set_state(tcb, THREAD_STATE_READY);

    /* setup scheduler */

    res = sched_init(tcb);
    if (res < 0) {
        console_perror("sched_init", -res);
        goto err_sched_init;
    }

    /* create and schedule system-service thread */

    res = tcb_helper_allocate_tcb_and_stack(task, 1, &tcb);
    if (res < 0) {
        console_perror("tcb_helper_allocate_tcb_and_stack", -res);
        goto err_tcb_helper_allocate_tcb_and_stack;
    }

    res = tcb_helper_run_kernel_thread(tcb, system_srv_start);
    if (res < 0) {
        console_perror("tcb_helper_run_kernel_thread", -res);
        goto err_tcb_helper_run_kernel_thread;
    }

    res = sched_add_thread(tcb, 255);
    if (res < 0) {
        console_perror("sched_add_service_thread", -res);
        goto err_sched_add_thread;
    }

    *task_out = task;

    return 0;

err_sched_add_thread:
err_tcb_helper_run_kernel_thread:
err_tcb_helper_allocate_tcb_and_stack:
err_sched_init:
err_tcb_helper_allocate_tcb:
    // TODO: clean up
    return res;
}

int
execute_module(struct task* parent, uintptr_t start, size_t len,
               const char* name)
{
    if (name) {
        console_printf("loading module '\%s'\n", name);
    } else {
        console_printf("loading module at address 0x%x\n", start);
    }

    /* allocate task */

    struct task* task;
    int res = task_helper_allocate_task_from_parent(parent, &task);

    if (res < 0) {
        console_perror("task_helper_allocate_task_from_parent", -res);
        goto err_task_helper_allocate_task_from_parent;
    }

    /* allocate TCB */

    struct tcb* tcb;
    res = tcb_helper_allocate_tcb_and_stack(task, 1, &tcb);

    if (res < 0) {
        console_perror("tcb_helper_allocate_tcb_and_stack" , -res);
        goto err_tcb_helper_allocate_tcb_and_stack;
    }

    /* load binary image */

    void* ip;
    res = loader_exec(tcb, (void *)start, &ip, tcb);

    if (res < 0) {
        console_perror("loader_exec", -res);
        goto err_loader_exec;
    }

    /* set thread to starting state */

    res = tcb_helper_run_user_thread(sched_get_current_thread(cpuid()),
                                     tcb, ip);
    if (res < 0) {
        goto err_tcb_helper_run_user_thread;
    }

    /* schedule thread */

    res = sched_add_thread(tcb, 64);

    if (res < 0) {
        console_perror("sched_add_thread", -res);
        goto err_sched_add_thread;
    }

    console_printf("scheduled as %x.\n", res);

    return 0;

err_sched_add_thread:
err_tcb_helper_run_user_thread:
err_loader_exec:
    /* FIXME: free tcb */
err_tcb_helper_allocate_tcb_and_stack:
    /* FIXME: free task */
err_task_helper_allocate_task_from_parent:
    return res;
}
