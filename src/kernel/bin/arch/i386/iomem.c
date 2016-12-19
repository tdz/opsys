/*
 *  opsys - A small, experimental operating system
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

#include "iomem.h"
#include <stddef.h>
#include <stdint.h>
#include "page.h"
#include "pageframe.h"
#include "pmem.h"
#include "vmem.h"
#include "vmemarea.h"

struct iomem {
    struct vmem* vmem;
};

static struct iomem g_iomem;

int
init_iomem(struct vmem* vmem)
{
    g_iomem.vmem = vmem;

    return 0;
}

void
uninit_iomem()
{ }

void*
map_io_range_nopg(const void* io_addr, const void* virt_addr, size_t length,
                  unsigned int flags)
{
    unsigned long pfindex = pageframe_index(io_addr);
    unsigned long pfcount = pageframe_span(io_addr, length);

    int res = pmem_claim_frames(pfindex, pfcount);
    if (res < 0) {
        return NULL;
    }

    os_index_t pgindex = page_index(virt_addr);

    res = vmem_map_pageframes_nopg(g_iomem.vmem,
                                   pfindex, pgindex, pfcount, flags);
    if (res < 0) {
        goto err_vmem_map_pageframes_nopg;
    }

    // offset of io_addr within it's pageframe
    unsigned long offset = ((uintptr_t)io_addr) - pageframe_offset(pfindex);

    return (void*)(page_offset(pgindex) + offset);

err_vmem_map_pageframes_nopg:
    pmem_unref_frames(pfindex, pfcount);
    return NULL;
}

void*
map_io_range(const void* io_addr, size_t length, unsigned int flags)
{
    unsigned long pfindex = pageframe_index(io_addr);
    unsigned long pfcount = pageframe_span(io_addr, length);

    const struct vmem_area* area = vmem_area_get_by_name(VMEM_AREA_KERNEL);

    os_index_t pgindex = vmem_empty_pages_within(g_iomem.vmem,
                                                 area->pgindex,
                                                 area->pgindex + area->npages,
                                                 pfcount);
    if (!pgindex) {
        return NULL;
    }

    int res = vmem_alloc_frames(g_iomem.vmem, pfindex, pfcount, pgindex, flags);

    if (res < 0) {
        return NULL;
    }

    // offset of phys_addr within it's pageframe
    unsigned long offset = ((uintptr_t)io_addr) - pageframe_offset(pfindex);

    return (void*)(page_offset(pgindex) + offset);
}

void
unmap_io_range(const void* virt_addr, size_t length)
{
    // todo: unmap
}
