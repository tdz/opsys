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
#include <sys/types.h>

#include "bitset.h"
#include "spinlock.h"
#include "task.h"

enum
{
        MAXTASK = 1024
};

static unsigned char g_taskid[MAXTASK >> 3];

int
task_init(struct task *task, struct vmem *as)
{
        int err;
        ssize_t taskid;

        if ((taskid = bitset_find_unset(g_taskid, sizeof(g_taskid))) < 0)
        {
                err = taskid;
                goto err_find_taskid;
        }

        bitset_set(g_taskid, taskid);

        task->as = as;
        task->nthreads = 0;
        task->id = taskid;
        memset(task->threadid, 0, sizeof(task->threadid));

        return 0;

err_find_taskid:
        return err;
}

void
task_uninit(struct task *task)
{
        bitset_unset(g_taskid, task->id);
}

size_t
task_max_nthreads(const struct task *task)
{
        return sizeof(task->threadid) * 8;
}

int
task_ref(struct task *task)
{
        if (!(task->nthreads < task_max_nthreads(task)))
        {
                return -EAGAIN;
        }

        return ++task->nthreads;
}

int
task_unref(struct task *task)
{
        if (!task->nthreads)
        {
                return -EINVAL;
        }

        return --task->nthreads;
}
