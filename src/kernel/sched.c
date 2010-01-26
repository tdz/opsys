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

#include <types.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

#include <cpu.h>

/* physical memory */
#include "pageframe.h"
#include "physmem.h"

/* virtual memory */
#include <page.h>
#include <pte.h>
#include <pagetbl.h>
#include <pde.h>
#include <pagedir.h>
#include "virtmem.h"

#include "task.h"
#include <tcb.h>
#include "sched.h"

enum {
        MAXTHREAD = 1024
};

static struct tcb*  g_thread[MAXTHREAD];
static unsigned int g_current_thread = 0;

int
sched_init()
{
        return 0;
}

ssize_t
sched_add_thread(struct tcb *tcb)
{
        ssize_t i = 0;

        while ((i < sizeof(g_thread)/sizeof(g_thread[0])) && g_thread[i]) {
                ++i;
        }

        if (i == sizeof(g_thread)/sizeof(g_thread[0])) {
                return -EAGAIN;
        }

        g_thread[i] = tcb;

        return i;
}

struct tcb *
sched_get_current_thread()
{
        return sched_get_thread(g_current_thread);
}

struct tcb *
sched_get_thread(unsigned int i)
{
        return g_thread[i];
}

#include "console.h"

static int
sched_switch_to(unsigned int i)
{
        int err;
        unsigned int current;

        current = g_current_thread;

        g_current_thread = i;

/*        console_printf("%s:%x src=%x dst=%x.\n", __FILE__, __LINE__,
                        g_thread[current], g_thread[i]);*/

        if ((err = tcb_switch(g_thread[current], g_thread[i])) < 0) {
                goto err_tcb_load;
        }
        
        return 0;

err_tcb_load:
        g_current_thread = current;
        return err;
}

int
sched_switch()
{
        int err;

        do {
                unsigned int next, i;

                next = g_current_thread+1;

                /* search up until end of list */

                for (i = next; i < sizeof(g_thread)/sizeof(g_thread[0]); ++i) {

                        if (!g_thread[i] || !tcb_is_runnable(g_thread[i])) {
                                continue;
                        }

                        if ((err = sched_switch_to(i)) < 0) {
                                goto err_sched_switch_to;
                        }

                        return 0;
                }

                /* search from beginning of list */

                for (i = 0; i < next; ++i) {

                        if (!g_thread[i] || !tcb_is_runnable(g_thread[i])) {
                                continue;
                        }

                        if ((err = sched_switch_to(i)) < 0) {
                                goto err_sched_switch_to;
                        }

                        return 0;
                }

                /* no thread ready; wait and search again after wakeup */
                hlt();
        } while (1);

err_sched_switch_to:
        return err;
}

void
sched_irq_handler(unsigned char irqno)
{
/*        console_printf("sched\n");*/
        sched_switch();
}

