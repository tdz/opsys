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

#include "stddef.h"
#include "types.h"
#include "page.h"
#include "pte.h"
#include "pagetbl.h"
#include "pde.h"
#include "pagedir.h"
#include "virtmem.h"
#include "tcb.h"
#include "task.h"

int
task_init(struct task *task, struct page_directory *pd)
{
        task->pd = pd;
        task->nthreads = 0;

        return 0;
}

void
task_uninit(struct task *task)
{
        return;
}

struct tcb *
task_get_tcb(struct task *task, unsigned char tcbid)
{
        return tcbid < 256 ? task->tcb+tcbid : NULL;
}

