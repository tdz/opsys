/*
 *  oskernel - A small experimental operating-system kernel
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
#include "pde.h"
#include "virtmem.h"
#include "tcb.h"

int
tcb_init(struct tcb *tcb)
{
        memset(tcb, 0, sizeof(*tcb));

        return 0;
}

int
tcb_save(struct tcb *tcb)
{
        return 0;
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
        
        phys_pgindex = page_directory_lookup_physical_page(pd,
                                        page_index((unsigned long)pd));

        tcb->cr3 = (phys_pgindex<<12) | (tcb->cr3&0xfff);

        return 0;
}

