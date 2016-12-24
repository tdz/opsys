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

#include "vmem.h"
#include <arch/i386/page.h>
#include <errno.h>
#include <stddef.h>
#include "cpu.h"
#include "pagedir.h"
#include "pageframe.h"
#include "pagetbl.h"
#include "pmem.h"
#include "pte.h"
#include "vmem_32.h"

/*
 * Public functions
 */

int
vmem_init(struct vmem* vmem,
          void* (alloc_aligned)(size_t, void*),
          void (*unref_aligned)(void*, size_t, void*), void* alloc_data)
{
    int res = semaphore_init(&vmem->sem, 1);
    if (res < 0) {
        return res;
    }

    res = vmem_32_init(&vmem->vmem_32, alloc_aligned, unref_aligned,
                       alloc_data);
    if (res < 0) {
        goto err_vmem_32_init;
    }

    return 0;

err_vmem_32_init:
    semaphore_uninit(&vmem->sem);
    return res;
}

static void*
alloc_pages(size_t siz, void* data)
{
    struct vmem* vmem = data;

    os_index_t pgindex = vmem_alloc_pages_in_area(vmem,
                                                  VMEM_AREA_KERNEL,
                                                  page_count(0, siz),
                                                  PTE_FLAG_PRESENT |
                                                  PTE_FLAG_WRITEABLE);
    if (pgindex < 0) {
        return NULL;
    }

    return page_address(pgindex);
}

static void
unref_pages(void* ptr, size_t siz, void* data)
{
    // TODO: unref pages
}

int
share_page_range(struct vmem* dst_vmem,
                 struct vmem* src_vmem,
                 os_index_t pgindex, size_t pgcount)
{
    return vmem_32_share_page_range(&dst_vmem->vmem_32,
                                    &src_vmem->vmem_32,
                                    pgindex, pgcount);
}

int
vmem_init_from_parent(struct vmem* vmem, struct vmem* parent)
{
    int res = vmem_init(vmem, alloc_pages, unref_pages, parent);
    if (res < 0) {
        return res;
    }

    for (enum vmem_area_name name = 0; name < LAST_VMEM_AREA; ++name) {

        const struct vmem_area* area = vmem_area_get_by_name(name);

        if (!(area->flags & VMEM_AREA_FLAG_GLOBAL)) {
            continue;
        }

        res = share_page_range(vmem, parent,
                               area->pgindex,
                               area->npages);
        if (res < 0) {
            goto err_share_page_range;
        }
    }

    return 0;

err_share_page_range:
    vmem_uninit(vmem);
    return res;
}

void
vmem_uninit(struct vmem* vmem)
{
    vmem_32_uninit(&vmem->vmem_32);
    semaphore_uninit(&vmem->sem);
}

int
vmem_alloc_frames(struct vmem *vmem, os_index_t pfindex, os_index_t pgindex,
                  size_t pgcount, unsigned int pteflags)
{
    semaphore_enter(&vmem->sem);

    int res = vmem_32_alloc_frames(&vmem->vmem_32,
                                   pfindex, pgindex, pgcount,
                                   pteflags);
    if (res < 0) {
        goto err_vmem_alloc_pageframes;
    }

    semaphore_leave(&vmem->sem);

    return 0;

err_vmem_alloc_pageframes:
    semaphore_leave(&vmem->sem);
    return res;
}

os_index_t
vmem_lookup_frame(struct vmem *vmem, os_index_t pgindex)
{
    semaphore_enter(&vmem->sem);

    os_index_t pfindex = vmem_32_lookup_frame(&vmem->vmem_32, pgindex);
    if (pfindex < 0) {
        goto err_vmem_lookup_pageframe;
    }

    semaphore_leave(&vmem->sem);

    return pfindex;

err_vmem_lookup_pageframe:
    semaphore_leave(&vmem->sem);
    return pfindex;
}

static size_t
check_pages_empty(struct vmem *vmem, os_index_t pgindex, size_t pgcount)
{
    return vmem_32_check_empty_pages(&vmem->vmem_32, pgindex, pgcount);
}

static os_index_t
find_empty_pages(struct vmem *vmem, size_t npages,
                 os_index_t pgindex_beg, os_index_t pgindex_end)
{
    /* find continuous area in virtual memory */

    while ((pgindex_beg < pgindex_end)
            && (npages < (pgindex_end - pgindex_beg))) {

        size_t nempty = check_pages_empty(vmem, pgindex_beg, npages);

        if (nempty == npages) {
            return pgindex_beg;
        }

        /* goto page after non-empty one */
        pgindex_beg += nempty + 1;
    }

    return -ENOMEM;
}

os_index_t
vmem_alloc_pages_at(struct vmem *vmem, os_index_t pgindex, size_t pgcount,
                    unsigned int pteflags)
{
    semaphore_enter(&vmem->sem);

    int res = vmem_32_alloc_pages(&vmem->vmem_32, pgindex, pgcount, pteflags);
    if (res < 0) {
        goto err_vmem_alloc_pages;
    }

    semaphore_leave(&vmem->sem);

    return 0;

err_vmem_alloc_pages:
    semaphore_leave(&vmem->sem);
    return res;
}

os_index_t
vmem_alloc_pages_within(struct vmem *vmem, os_index_t pgindex_min,
                        os_index_t pgindex_max, size_t npages,
                        unsigned int pteflags)
{
    semaphore_enter(&vmem->sem);

    int res;

    os_index_t pgindex = find_empty_pages(vmem, npages,
                                          pgindex_min, pgindex_max);
    if (pgindex < 0) {
        res = pgindex;
        goto err_vmem_find_empty_pages;
    }

    res = vmem_32_alloc_pages(&vmem->vmem_32, pgindex, npages, pteflags);
    if (res < 0) {
        goto err_vmem_alloc_pages;
    }

    semaphore_leave(&vmem->sem);

    return pgindex;

err_vmem_alloc_pages:
err_vmem_find_empty_pages:
    semaphore_leave(&vmem->sem);
    return res;
}

os_index_t
vmem_alloc_pages_in_area(struct vmem* vmem, enum vmem_area_name areaname,
                         size_t npages, unsigned int pteflags)
{
    const struct vmem_area* area = vmem_area_get_by_name(areaname);

    return vmem_alloc_pages_within(vmem,
                                   area->pgindex,
                                   area->pgindex + area->npages, npages,
                                   pteflags);
}

static void
semaphore_enter_ordered(struct semaphore *sem1, struct semaphore *sem2)
{
    if (sem1 < sem2) {
        semaphore_enter(sem1);
        semaphore_enter(sem2);
    } else if (sem1 > sem2) {
        semaphore_enter(sem2);
        semaphore_enter(sem1);
    } else {    /* sem1 == sem2 */
        semaphore_enter(sem1);
    }
}

static void
semaphore_leave_ordered(struct semaphore *sem1, struct semaphore *sem2)
{
    if (sem1 != sem2) {
        semaphore_leave(sem1);
        semaphore_leave(sem2);
    } else {    /* sem1 == sem2 */
        semaphore_leave(sem1);
    }
}

int
vmem_map_pages_at(struct vmem *dst_vmem, os_index_t dst_pgindex,
                  struct vmem *src_vmem, os_index_t src_pgindex,
                  size_t pgcount, unsigned long pteflags)
{
    semaphore_enter_ordered(&dst_vmem->sem, &src_vmem->sem);

    int res = vmem_32_map_pages(&dst_vmem->vmem_32, dst_pgindex,
                                &src_vmem->vmem_32, src_pgindex,
                                pgcount, pteflags);
    if (res < 0) {
        goto err_vmem_map_pages;
    }

    semaphore_leave_ordered(&dst_vmem->sem, &src_vmem->sem);

    return 0;

err_vmem_map_pages:
    semaphore_leave_ordered(&dst_vmem->sem, &src_vmem->sem);
    return res;
}

os_index_t
vmem_map_pages_within(struct vmem *dst_vmem, os_index_t pg_index_min,
                      os_index_t pg_index_max, struct vmem *src_vmem,
                      os_index_t src_pgindex, size_t pgcount,
                      unsigned long dst_pteflags)
{
    semaphore_enter_ordered(&dst_vmem->sem, &src_vmem->sem);

    int res;

    os_index_t dst_pgindex = find_empty_pages(dst_vmem, pgcount,
                                              pg_index_min,
                                              pg_index_max);
    if (dst_pgindex < 0) {
        res = dst_pgindex;
        goto err_find_empty_pages;
    }

    res = vmem_32_map_pages(&dst_vmem->vmem_32, dst_pgindex,
                            &src_vmem->vmem_32, src_pgindex,
                            pgcount, dst_pteflags);
    if (res < 0) {
        goto err_vmem_map_pages;
    }

    semaphore_leave_ordered(&dst_vmem->sem, &src_vmem->sem);

    return dst_pgindex;

err_vmem_map_pages:
err_find_empty_pages:
    semaphore_leave_ordered(&dst_vmem->sem, &src_vmem->sem);
    return res;
}

os_index_t
vmem_map_pages_in_area(struct vmem* dst_vmem, enum vmem_area_name dst_areaname,
                       struct vmem* src_vmem,
                       os_index_t src_pgindex, size_t pgcount,
                       unsigned long dst_pteflags)
{
    const struct vmem_area* dst_area = vmem_area_get_by_name(dst_areaname);

    return vmem_map_pages_within(dst_vmem,
                                 dst_area->pgindex,
                                 dst_area->pgindex + dst_area->npages,
                                 src_vmem,
                                 src_pgindex, pgcount,
                                 dst_pteflags);
}

os_index_t
vmem_empty_pages_within(struct vmem *vmem, os_index_t pg_index_min,
                        os_index_t pg_index_max, size_t pgcount)
{
    semaphore_enter(&vmem->sem);

    int res;

    os_index_t pgindex = find_empty_pages(vmem, pgcount,
                                          pg_index_min,
                                          pg_index_max);
    if (pgindex < 0) {
        res = pgindex;
        goto err_vmem_find_empty_pages;
    }

    semaphore_leave(&vmem->sem);

    return pgindex;

err_vmem_find_empty_pages:
    semaphore_leave(&vmem->sem);
    return res;
}

os_index_t
vmem_empty_pages_in_area(struct vmem* vmem,
                         enum vmem_area_name areaname, size_t pgcount)
{
    const struct vmem_area* area = vmem_area_get_by_name(areaname);

    return vmem_empty_pages_within(vmem, area->pgindex,
                                         area->pgindex + area->npages,
                                         pgcount);
}

/*
 * Public functions for Protected Mode setup
 */

static void*
alloc_frames(size_t siz, void* data)
{
    unsigned long pfcount = pageframe_count(siz);

    unsigned long pfindex = pmem_alloc_frames(pfcount);
    if (!pfindex) {
        return NULL;
    }
    return (void*)(uintptr_t)pageframe_offset(pfindex);
}

static void
unref_frames(void* ptr, size_t siz, void* data)
{
    pmem_unref_frames(pageframe_index(ptr),
                      pageframe_span(ptr, siz));
}

int
vmem_init_nopg(struct vmem* vmem)
{
    /* init vmem data structure for kernel task */

    int res = vmem_init(vmem, alloc_frames, unref_frames, NULL);
    if (res < 0) {
        return res;
    }

    /* install page tables in all kernel areas */

    enum vmem_area_name name;

    for (name = 0; name < LAST_VMEM_AREA; ++name) {

        const struct vmem_area* area = vmem_area_get_by_name(name);

        if (!(area->flags & VMEM_AREA_FLAG_PAGETABLES)) {
            continue;
        }

        unsigned long ptindex = pagetable_index(page_address(area->pgindex));
        unsigned long ptcount = pagetable_count(page_address(area->pgindex),
                                                page_memory(area->npages));

        res = vmem_alloc_page_tables_nopg(vmem, ptindex, ptcount,
                                          PDE_FLAG_PRESENT|
                                          PDE_FLAG_WRITEABLE);

        if (res < 0) {
            goto err_vmem_alloc_page_tables_nopg;
        }
    }

    /* create identity mapping for all identity areas */

    for (name = 0; name < LAST_VMEM_AREA; ++name) {

        const struct vmem_area *area = vmem_area_get_by_name(name);

        if (!(area->flags & VMEM_AREA_FLAG_POLUTE)) {
            continue;
        } else if (!(area->flags & VMEM_AREA_FLAG_IDENTITY)) {
            continue;
        }

        res = vmem_map_pageframes_nopg(vmem,
                                       area->pgindex,
                                       area->pgindex,
                                       area->npages,
                                       PTE_FLAG_PRESENT |
                                       PTE_FLAG_WRITEABLE);
        if (res < 0) {
            goto err_vmem_map_pageframes_nopg_identity;
        }
    }

    /* populate remaining areas */

    for (name = 0; name < LAST_VMEM_AREA; ++name) {

        const struct vmem_area *area = vmem_area_get_by_name(name);

        if (!(area->flags & VMEM_AREA_FLAG_POLUTE)) {
            continue;
        } else if (area->flags & VMEM_AREA_FLAG_IDENTITY) {
            continue;
        }

        os_index_t pfindex =
            pmem_alloc_frames(pageframe_count(page_memory(area->npages)));

        if (!pfindex) {
            res = -ENOMEM;
            break;
        }

        res = vmem_map_pageframes_nopg(vmem, pfindex,
                                       area->pgindex,
                                       area->npages,
                                       PTE_FLAG_PRESENT |
                                       PTE_FLAG_WRITEABLE);
        if (res < 0) {
            goto err_vmem_map_pageframes_nopg_pollute;
        }
    }

    /* prepare temporary mappings */
    res = vmem_install_tmp_nopg(vmem);
    if (res < 0) {
        goto err_vmem_install_tmp_nopg;
    }

    /* map page directory */
    res = vmem_map_paging_structures_nopg(vmem);
    if (res < 0){
        goto err_vmem_map_paging_structures_nopg;
    }

    return 0;

err_vmem_map_paging_structures_nopg:
err_vmem_install_tmp_nopg:
err_vmem_map_pageframes_nopg_pollute:
err_vmem_map_pageframes_nopg_identity:
err_vmem_alloc_page_tables_nopg:
    vmem_uninit(vmem);
    return res;
}


int
vmem_map_pageframes_nopg(struct vmem* vmem, os_index_t pfindex,
                         os_index_t pgindex, size_t count, unsigned int flags)
{
    os_index_t pfbeg = pfindex;
    os_index_t pfend = pfbeg + count;
    os_index_t pgbeg = pgindex;

    while (pfbeg < pfend) {
        int res = vmem_32_map_pageframe_nopg(&vmem->vmem_32,
                                             pfbeg, pgbeg,
                                             flags);
        if (res < 0) {
            return res; // no undo; simply fail
        }

        ++pgbeg;
        ++pfbeg;
    }

    return 0;
}

int
vmem_alloc_page_tables_nopg(struct vmem* vmem, os_index_t ptindex,
                            size_t ptcount, unsigned int flags)
{
    os_index_t ptbeg = ptindex;
    os_index_t ptend = ptbeg + ptcount;

    while (ptbeg < ptend) {
        int res = vmem_32_alloc_page_table_nopg(&vmem->vmem_32, ptbeg, flags);

        if (res < 0) {
            return res; // no undo; simply fail
        }

        ++ptbeg;
    }

    return 0;
}

int
vmem_install_tmp_nopg(struct vmem* vmem)
{
    int res = vmem_32_install_tmp_nopg(&vmem->vmem_32);
    if (res < 0) {
        return res;
    }
    return 0;
}

int
vmem_map_paging_structures_nopg(struct vmem* vmem)
{
    int res = vmem_32_map_paging_structures_nopg(&vmem->vmem_32);
    if (res < 0) {
        return res;
    }
    return 0;
}

void
vmem_enable_paging_nopg(struct vmem* vmem)
{
    vmem_32_enable_paging_nopg(&vmem->vmem_32);
}

/*
 * Fault handlers
 */

#include "console.h"

void
vmem_segfault_handler(void *ip)
{
    console_printf("segmentation fault: ip=%x.\n", (unsigned long)ip);
}

void
vmem_pagefault_handler(void *ip, void *addr, unsigned long errcode)
{
    console_printf("page fault: ip=%x, addr=%x, errcode=%x.\n",
                    (unsigned long)ip,
                    (unsigned long)addr, (unsigned long)errcode);

    while (1) {
        hlt();
    }
}
