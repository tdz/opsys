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

#include "main.h"
#include "console.h"
#include "cpu.h"
#include "drivers/i8042/kbd.h"
#include "drivers/i8254/i8254.h"
#include "drivers/i8259/pic.h"
#include "gdt.h"
#include "idt.h"
#include "interupt.h"
#include "loader.h"
#include "sched.h"
#include "syscall.h"
#include "syssrv.h"
#include "taskhlp.h"
#include "tcb.h"
#include "tcbhlp.h"
#include "vmem.h"

/*
 * Platform drivers
 */

static struct i8254_drv g_i8254_drv;

/*
 * Platform entry points for ISR handlers
 *
 * The platform_ functions below are the entry points from the
 * IDT's interupt handlers into the main executable. The functions
 * must forward the interupts to whatever drivers or modules have
 * been initialized.
 */

void __attribute__((used))
platform_handle_invalid_opcode(void* ip)
{
    console_printf("invalid opcode ip=%x.\n", (unsigned long)ip);
}

void __attribute__((used))
platform_handle_irq(unsigned char irqno)
{
    pic_handle_irq(irqno);
}

void __attribute__((used))
platform_eoi(unsigned char irqno)
{
    pic_eoi(irqno);
}

void __attribute__((used))
platform_handle_segmentation_fault(void* ip)
{
    vmem_segfault_handler(ip);
}

void __attribute__((used))
platform_handle_page_fault(void* ip, void* addr, unsigned long errcode)
{
    vmem_pagefault_handler(ip, addr, errcode);
}

void __attribute__((used))
platform_handle_syscall(unsigned long* r0, unsigned long* r1,
                        unsigned long* r2, unsigned long* r3)
{
    syscall_entry_handler(r0, r1, r2, r3);
}

/**
 * \brief sets up virtual memory and system services
 * \param[in, out] tsk return the kernel task
 * \param[in] vmem kernel-task virtual address space
 * \param[in] stack initial address of the idle thread's stack
 * \return 0 on success, or a negative error code otherwise
 */
int
general_init(struct task **tsk, struct vmem* vmem, void *stack)
{
        int err;
        struct tcb *tcb;

        /*
         * setup GDT for protected mode
         */
        gdt_init();
        gdt_install();

        /* setup IDT for protected mode */
        init_idt();

        /*
         * setup interupt controller
         */
        pic_install();

        /*
         * setup keyboard
         */
        if ((err = kbd_init()) < 0)
        {
                console_perror("kbd_init", -err);
        }

        /* setup PIT for system timer */

        int res = i8254_init(&g_i8254_drv);
        if (res < 0) {
            goto err_i8254_init;
        }

        i8254_install_timer(&g_i8254_drv, SCHED_FREQ);

        /*
         * build initial task and address space
         */

        if ((err = task_helper_init_kernel_task(vmem, tsk)) < 0)
        {
                console_perror("task_helper_init_kernel_task", -err);
                goto err_task_helper_init_kernel_task;
        }

        /*
         * setup current execution as thread 0 of kernel task
         */

        if ((err = tcb_helper_allocate_tcb(*tsk, stack, &tcb)) < 0)
        {
                console_perror("tcb_helper_allocate_tcb", -err);
                goto err_tcb_helper_allocate_tcb;
        }

        tcb_set_state(tcb, THREAD_STATE_READY);

        /*
         * setup scheduler
         */

        if ((err = sched_init(tcb)) < 0)
        {
                console_perror("sched_init", -err);
                goto err_sched_init;
        }

/*
        if ((err = sched_add_thread(tcb, 0)) < 0)
        {
                console_perror("sched_add_idle_thread", -err);
                goto err_sched_add_idle_thread;
        }*/

        sti();

        /*
         * create and schedule system service
         */

        if ((err = tcb_helper_allocate_tcb_and_stack(*tsk, 1, &tcb)) < 0)
        {
                console_perror("tcb_helper_allocate_tcb_and_stack", -err);
                goto err_tcb_helper_allocate_tcb_and_stack;
        }

        if ((err = tcb_helper_run_kernel_thread(tcb, system_srv_start)) < 0)
        {
                console_perror("tcb_set_initial_ready_state", -err);
                goto err_tcb_set_initial_ready_state;
        }

        if ((err = sched_add_thread(tcb, 255)) < 0)
        {
                console_perror("sched_add_service_thread", -err);
                goto err_sched_add_service_thread;
        }

        return 0;

err_sched_add_service_thread:
err_tcb_set_initial_ready_state:
err_tcb_helper_allocate_tcb_and_stack:
        cli();
err_sched_init:
err_tcb_helper_allocate_tcb:
err_task_helper_init_kernel_task:
err_i8254_init:
        uninit_console();
        return err;
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
