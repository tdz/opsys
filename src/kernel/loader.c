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

#include <errno.h>
#include <types.h>

#include <cpu.h>

#include "pde.h"
#include "pagedir.h"
#include "tcb.h"
#include "task.h"
#include "elfldr.h"
#include "loader.h"

static int
loader_prepare_tcb(struct tcb *tcb, const void *ip)
{
        extern void tcb_switch_to_starting_return_address(void);
        extern void isr_handle_irq_return(void);
        extern void tcb_ret(void);

        unsigned long *stack;

        /* prepare stack as if tcb was scheduled from irq0 */

        stack = tcb->stack;

        /* stack after irq 0 */
        stack[-1] = eflags();
        stack[-2] = /*cs();*/ 8;
        stack[-3] = (unsigned long)ip;
        stack[-4] = 0; /* irqno */

        stack[-5] = 0; /* %eax */
        stack[-6] = 0; /* %ecx */
        stack[-7] = 0; /* %edx */
        stack[-8] = 0; /* irqno */ /* pushed by irq handler */

        /* tcb_switch */
        stack[-9] = (unsigned long)tcb;
        stack[-10] = (unsigned long)tcb;
        stack[-11] = (unsigned long)isr_handle_irq_return; /* from irq handler */
        stack[-11] = (unsigned long)tcb_ret; /* from irq handler */
        stack[-12] = (unsigned long)(stack-11);

        tcb->cr0 = cr0();
        tcb->cr2 = cr2();
/*        tcb->cr3 = cr3();*/
        tcb->cr4 = cr4();
        tcb->eflags = eflags();
        tcb->esp = (unsigned long)(stack-12);
        tcb->ebp = (unsigned long)(stack-11);

        tcb_set_ip(tcb, tcb_switch_to_starting_return_address);

        tcb_set_state(tcb, THREAD_STATE_STARTING);

        return 0;
}

int
loader_exec(struct tcb *tcb, const void *img)
{
        int err;
        void *ip;

        /* load image into thread */

        if (elf_loader_is_elf(img)) {
                err = elf_loader_exec(tcb, &ip, img);
        } else {
                err = -EINVAL;
        }

        if (err < 0) {
                goto err_is_image;
        }

        /* set thread to starting state */

        loader_prepare_tcb(tcb, ip);

        return 0;

err_is_image:
        return err;
}

