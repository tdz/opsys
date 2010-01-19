/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "types.h"
#include "string.h"
#include "page.h"
#include "pte.h"
#include "pagetbl.h"
#include "pde.h"
#include "pagedir.h"
#include "virtmem.h"
#include "tcb.h"

int
tcb_init(struct tcb *tcb)
{
        memset(tcb, 0, sizeof(*tcb));

        return 0;
}

void
tcb_save(struct tcb *tcb, unsigned long ip, unsigned long eflags)
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
        tcb->eflags = eflags;
}

int
tcb_load(const struct tcb *tcb)
{
        __asm__("movl %0, %%cr3\n\t"
                        :
                        : "r"(tcb->cr3)
                        :);

        return 0;
}

int
tcb_set_page_directory(struct tcb *tcb, struct page_directory *pd)
{
        unsigned long phys_pgindex;

        phys_pgindex = virtmem_lookup_physical_page(pd,
                                        page_index((unsigned long)pd));

        tcb->cr3 = (phys_pgindex<<12) | (tcb->cr3&0xfff);

        return 0;
}

