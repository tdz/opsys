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

#include "pde.h"
#include "pagedir.h"
#include "tcb.h"
#include "task.h"
#include "elfldr.h"
#include "loader.h"

int
loader_exec(struct task *tsk, const void *img)
{
        if (elf_loader_is_elf(img)) {
                return elf_loader_exec(tsk, img);
        }

        return -EINVAL;
}
