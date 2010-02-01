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

#include "tcb.h"
#include "task.h"
#include "elfldr.h"
#include "loader.h"

int
loader_exec(struct tcb *tcb, const void *img)
{
        int err;
        void *ip;

        /* load image into thread */

        if (elf_loader_is_elf(img)) {
                err = elf_loader_exec(tcb->task->pd, &ip, img);
        } else {
                err = -EINVAL;
        }

        if (err < 0) {
                goto err_is_image;
        }

        /* set thread to starting state */

        tcb_set_initial_ready_state(tcb, ip, 0);

        return 0;

err_is_image:
        return err;
}

