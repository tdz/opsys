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

#include "loader.h"
#include <errno.h>
#include <sys/types.h>
#include "elfldr.h"
#include "task.h"
#include "tcb.h"

int
loader_exec(const struct tcb *tcb,
            const void *img, void **ip, struct tcb *dst_tcb)
{
        int err;

        /*
         * load image into address space
         */

        if (elf_loader_is_elf(img))
        {
                err = elf_loader_exec(tcb->task->as, img, ip,
                                      dst_tcb->task->as);
        }
        else
        {
                err = -EINVAL;
        }

        if (err < 0)
        {
                goto err_is_image;
        }

        return 0;

err_is_image:
        return err;
}
