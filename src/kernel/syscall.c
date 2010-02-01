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
#include <sys/types.h>

#include "tid.h"
#include "sched.h"
#include "syscall.h"

#include "console.h"

int
syscall_entry_handler(unsigned long r0,
                      unsigned long r1,
                      unsigned long r2,
                      unsigned long r3)
{
        int err;
        struct tcb *snd, *rcv;

        console_printf("%s:%x: r0=%x r1=%x r2=%x r3=%x.\n", __FILE__, __LINE__,
                        r0, r1, r2, r3);

        /* get current thread */

        if (!(snd = sched_get_current_thread())) {
                err = -EAGAIN;
                goto err_sched_get_current_thread;
        }

        /* get receiver thread */

        rcv = sched_search_thread(threadid_get_taskid(r0),
                                  threadid_get_tcbid(r0));
        if (!rcv) {
                err = -EAGAIN;
                goto err_sched_search_thread;
        }

        /* create IPC message */

        return 0;

err_sched_search_thread:
err_sched_get_current_thread:
        return err;
}

