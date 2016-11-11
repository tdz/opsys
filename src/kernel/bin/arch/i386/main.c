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
#include "alloc.h"
#include "console.h"
#include "cpu.h"
#include "gdt.h"
#include "idt.h"
#include "interupt.h"
#include "kbd.h"
#include "pic.h"
#include "pit.h"
#include "sched.h"
#include "syscall.h"
#include "syssrv.h"
#include "taskhlp.h"
#include "tcb.h"
#include "tcbhlp.h"
#include "vmem.h"

enum
{
        SCHED_FREQ = 20 /**< \brief frequency for thread scheduling */
};

static void
main_invalop_handler(void *ip)
{
        console_printf("invalid opcode ip=%x.\n", (unsigned long)ip);
}

/**
 * \brief sets up virtual memory and system services
 * \param[in, out] tsk return the kernel task
 * \param[in] stack initial address of the idle thread's stack
 * \return 0 on success, or a negative error code otherwise
 */
int
general_init(struct task **tsk, void *stack)
{
        static struct vmem g_kernel_as;

        int err;
        struct tcb *tcb;

        /*
         * setup GDT for protected mode
         */
        gdt_init();
        gdt_install();

        /*
         * setup IDT for protected mode
         */
        idt_init();
        idt_install();

        idt_install_invalid_opcode_handler(main_invalop_handler);

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
        else
        {
                idt_install_irq_handler(1, kbd_irq_handler);
        }

        /*
         * setup PIT for system timer
         */
        pit_install(0, SCHED_FREQ, PIT_MODE_RATEGEN);
        idt_install_irq_handler(0, pit_irq_handler);

        idt_install_syscall_handler(syscall_entry_handler);

        /*
         * build initial task and address space
         */

        idt_install_segfault_handler(vmem_segfault_handler);
        idt_install_pagefault_handler(vmem_pagefault_handler);

        if ((err = task_helper_init_kernel_task(&g_kernel_as, tsk)) < 0)
        {
                console_perror("task_helper_init_kernel_task", -err);
                goto err_task_helper_init_kernel_task;
        }

        /*
         * setup memory allocator
         */

        if ((err = allocator_init(&g_kernel_as)) < 0)
        {
                console_perror("allocator_init", -err);
                goto err_allocator_init;
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

        if ((err = sched_init(cpuid(), tcb)) < 0)
        {
                console_perror("sched_init", -err);
                goto err_sched_init;
        }

        idt_install_irq_handler(0, sched_irq_handler);

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
err_allocator_init:
err_task_helper_init_kernel_task:
        return err;
}

