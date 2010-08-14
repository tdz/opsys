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
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#include <minmax.h>
#include "membar.h"
#include "mmu.h"
#include "cpu.h"

#include <spinlock.h>
#include <semaphore.h>

/* physical memory */
#include "pageframe.h"
#include <physmem.h>

/* virtual memory */
#include "page.h"
#include "pte.h"
#include "pagetbl.h"
#include "pde.h"
#include "pagedir.h"
#include <vmemarea.h>

#include "vmem.h"
#include "vmem_pae.h"

int
vmem_pae_map_pageframe_nopg(void *tlps,
                                     os_index_t pfindex,
                                     os_index_t pgindex, unsigned int flags)
{
        return -ENOSYS;
}

int
vmem_pae_alloc_page_table_nopg(void *tlps,
                                        os_index_t ptindex,
                                        unsigned int flags)
{
        return -ENOSYS;
}

void
vmem_pae_enable(const void *tlps)
{
        return;
}

size_t
vmem_pae_check_empty_pages(const void *tlps, os_index_t pgindex,
                           size_t pgcount)
{
        return -ENOSYS;
}

int
vmem_pae_alloc_frames(void *tlps, os_index_t pfindex, os_index_t pgindex,
                      size_t pgcount, unsigned int pteflags)
{
        return -ENOSYS;
}

os_index_t
vmem_pae_lookup_frame(const void *tlps, os_index_t pgindex)
{
        return -ENOSYS;
}

int
vmem_pae_alloc_pages(void *tlps, os_index_t pgindex, size_t pgcount,
                     unsigned int pteflags)
{
        return -ENOSYS;
}

int
vmem_pae_map_pages(void *dst_tlps,
                            os_index_t dst_pgindex,
                            const struct vmem *src_as,
                            os_index_t src_pgindex,
                            size_t pgcount, unsigned long dst_pteflags)
{
        return -ENOSYS;
}

int
vmem_pae_share_2nd_lvl_ps(void *dst_tlps,
                                   const void *src_tlps,
                                   os_index_t pgindex, size_t pgcount)
{
        return -ENOSYS;
}

