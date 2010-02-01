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

#include <string.h>
#include <sys/types.h>

#include "pageframe.h"
#include "physmem.h"

#include "pde.h"
#include "pagedir.h"

int
page_directory_init(struct page_directory *pd)
{
        memset(pd, 0, sizeof(*pd));

        return 0;
}

void
page_directory_uninit(struct page_directory *pd)
{
        return;
}

int
page_directory_install_page_table(struct page_directory *pd,
                                  unsigned long pfindex,
                                  unsigned long index,
                                  unsigned int flags)
{
        int err;

        /* ref new page table's page frame */

        if ((err = physmem_ref_frames(pfindex, 1)) < 0) {
                goto err_physmem_ref_frames;
        }

        /* unref old page table's page frame */

        if (pde_get_pageframe_index(pd->entry[index])) {
                physmem_unref_frames(
                        pde_get_pageframe_index(pd->entry[index]), 1);
        }

        /* update page directory entry */
        pd->entry[index] = pde_create(pfindex, flags);

        return 0;

err_physmem_ref_frames:
        return err;
}

int
page_directory_install_page_tables(struct page_directory *pd,
                                   unsigned long pfindex,
                                   unsigned long index,
                                   unsigned long count,
                                   unsigned int flags)
{
        int err;

        for (err = 0; count && !(err < 0); --count, ++index, ++pfindex) {
                err = page_directory_install_page_table(pd,
                                                        pfindex,
                                                        index,
                                                        flags);
        }

        return err;
}

int
page_directory_uninstall_page_table(struct page_directory *pd,
                                    unsigned long index)
{
        /* unref page frame of page-table */

        if (pde_get_pageframe_index(pd->entry[index])) {
                physmem_unref_frames(
                        pde_get_pageframe_index(pd->entry[index]), 1);
        }

        /* clear page directory entry */
        pd->entry[index] = pde_create(0, 0);

        return 0;
}

int
page_directory_uninstall_page_tables(struct page_directory *pd,
                                     unsigned long index,
                                     unsigned long count)
{
        int err;

        for (err = 0; count && !(err < 0); --count, ++index) {
                err = page_directory_uninstall_page_table(pd, index);
        }

        return err;
}

