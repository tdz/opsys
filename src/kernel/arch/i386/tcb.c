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
#include <string.h>
#include <types.h>

#include "bitset.h"

#include "interupt.h"

/* virtual memory */
#include "page.h"
#include "pte.h"
#include "pagetbl.h"
#include "pde.h"
#include "pagedir.h"
#include "virtmem.h"

#include "task.h"
#include "tcb.h"

int
tcb_init_with_id(struct tcb *tcb,
                 struct task *task, unsigned char id, void *stack)
{
        int err;

        if ((err = task_ref(task)) < 0) {
                goto err_task_ref;
        }

        if (bitset_isset(task->threadid, id)) {
                err = -EINVAL;
                goto err_bitset_isset;
        }

        bitset_set(task->threadid, id);

        memset(tcb, 0, sizeof(*tcb));

        tcb->state = THREAD_STATE_ZOMBIE;
        tcb->task = task;
        tcb->stack = stack;
        tcb->id = id;

        tcb->esp = (unsigned long)tcb->stack;

        tcb_set_page_directory(tcb, tcb->task->pd);

        return 0;

err_bitset_isset:
        task_unref(task);
err_task_ref:
        return err;
}

int
tcb_init(struct tcb *tcb, struct task *task, void *stack)
{
        int err;
        ssize_t id;

        id = bitset_find_unset(task->threadid, sizeof(task->threadid));

        if (id < 0) {
                err = id;
                goto err_bitset_find_unset;
        }

        return tcb_init_with_id(tcb, task, id, stack);

err_bitset_find_unset:
        return err;
}

void
tcb_uninit(struct tcb *tcb)
{
        task_unref(tcb->task);
}

void
tcb_set_state(struct tcb *tcb, enum thread_state state)
{
        tcb->state = state;
}

enum thread_state
tcb_get_state(const struct tcb *tcb)
{
        return tcb->state;
}

void
tcb_set_ip(struct tcb *tcb, void *ip)
{
        tcb->eip = (unsigned long)ip;
}

int
tcb_switch_to_starting(struct tcb *tcb, struct tcb *dst);

int
tcb_switch_to_ready(struct tcb *tcb, struct tcb *dst);

static int
tcb_switch_to_blocked(struct tcb *tcb, struct tcb *dst)
{
        return 0;
}

static int
tcb_switch_to_zombie(struct tcb *tcb, struct tcb *dst)
{
        return 0;
}

int
tcb_switch(struct tcb *tcb, struct tcb *dst)
{
        static int (* const switch_to[])(struct tcb*, struct tcb*) = {
                tcb_switch_to_starting,
                tcb_switch_to_ready,
                tcb_switch_to_blocked,
                tcb_switch_to_zombie};

        return switch_to[dst->state](tcb, dst);
}

#ifdef COMMENT
int
tcb_switch(struct tcb *tcb, struct tcb *dst)
{
        cli();

        __asm__(/* save %eax first, as it later contains the tcb address */
                "pushl %%edx\n\t"
                "lea 8(%%ebp), %%edx\n\t"
                "movl %%eax, (%%edx)\n\t"
                "movl %%edx, %%eax\n\t"
                "popl %%edx\n\t"
                /* adjust %eax to point to first register in tcb */
                "addl $16, %%eax\n\t"
                /* save other general-purpose registers */
                "movl %%ebx, 4(%%eax)\n\t"
                "movl %%ecx, 8(%%eax)\n\t"
                "movl %%edx, 12(%%eax)\n\t"
                /* index registers */
                "movl %%esi, 16(%%eax)\n\t"
                "movl %%edi, 20(%%eax)\n\t"
                "movl %%ebp, 24(%%eax)\n\t"
                "movl %%esp, 32(%%eax)\n\t"
                /* segmentation registers */
                "mov %%cs, 36(%%eax)\n\t"
                "mov %%ds, 38(%%eax)\n\t"
                "mov %%ss, 40(%%eax)\n\t"
                "mov %%es, 42(%%eax)\n\t"
                "mov %%fs, 44(%%eax)\n\t"
                "mov %%gs, 46(%%eax)\n\t"
                /* control registers (need intermediate register) */
                "movl %%cr0, %%ebx\n\t" "movl %%ebx, 48(%%eax)\n\t"
                "movl %%cr2, %%ebx\n\t" "movl %%ebx, 52(%%eax)\n\t"
                "movl %%cr3, %%ebx\n\t" "movl %%ebx, 56(%%eax)\n\t"
                "movl %%cr4, %%ebx\n\t" "movl %%ebx, 60(%%eax)\n\t"
                /* push flags onto stack */
/*                "pushf\n\t"*/
                        :
                        :
                        : "%eax" /* used for tcb address */);

        __asm__("movl %0, %%cr3\n\t"
                        :
                        : "r"(dst->cr3)
                        :);

        sti();

        __asm__("jmp *%0\n\t"
                        :
                        : "r"(dst->eip) );

        return 0;
}
#endif
void
tcb_save(struct tcb *tcb, unsigned long ip)
{
        __asm__(/* save %eax first, as it later contains the tcb address */
                "pushl %%edx\n\t"
                "lea 8(%%ebp), %%edx\n\t"
                "movl %%eax, (%%edx)\n\t"
                "movl %%edx, %%eax\n\t"
                "popl %%edx\n\t"
                /* save other general-purpose registers */
                "movl %%ebx, 4(%%eax)\n\t"
                "movl %%ecx, 8(%%eax)\n\t"
                "movl %%edx, 12(%%eax)\n\t"
                /* index registers */
                "movl %%esi, 16(%%eax)\n\t"
                "movl %%edi, 20(%%eax)\n\t"
                "movl %%ebp, 24(%%eax)\n\t"
                "movl %%esp, 32(%%eax)\n\t"
                /* segmentation registers */
                "mov %%cs, 36(%%eax)\n\t"
                "mov %%ds, 38(%%eax)\n\t"
                "mov %%ss, 40(%%eax)\n\t"
                "mov %%es, 42(%%eax)\n\t"
                "mov %%fs, 44(%%eax)\n\t"
                "mov %%gs, 46(%%eax)\n\t"
                /* control registers (need intermediate register) */
                "movl %%cr0, %%ebx\n\t" "movl %%ebx, 48(%%eax)\n\t"
                "movl %%cr2, %%ebx\n\t" "movl %%ebx, 52(%%eax)\n\t"
                "movl %%cr3, %%ebx\n\t" "movl %%ebx, 56(%%eax)\n\t"
                "movl %%cr4, %%ebx\n\t" "movl %%ebx, 60(%%eax)\n\t"
                /* instruction pointer and flags come from outside */
                        :
                        :
                        : "%eax" /* used for tcb address */);

        tcb->eip = ip;
/*        tcb->eflags = eflags;*/
}

int
tcb_load(const struct tcb *tcb)
{
        __asm__("movl %0, %%cr3\n\t"
                        :
                        : "r"(tcb->cr3)
                        :);

        __asm__("jmp *%0\n\t"
                        :
                        : "r"(tcb->eip) );

        return 0;
}

int
tcb_set_page_directory(struct tcb *tcb, const struct page_directory *pd)
{
        os_index_t pfindex;

        pfindex = virtmem_lookup_pageframe(pd, page_index(pd));

        tcb->cr3 = (pfindex<<12) | (tcb->cr3&0xfff);

        return 0;
}

