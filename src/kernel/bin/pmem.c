/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009-2010  Thomas Zimmermann
 *  Copyright (C) 2016       Thomas Zimmermann
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

#include "pmem.h"
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include "pageframe.h"
#include "semaphore.h"

//
// pmem_map_t helpers
//

static pmem_map_t
set_type(pmem_map_t memmap, enum pmem_type type)
{
    return (memmap & 0x3f) | type;
}

static enum pmem_type
get_type(pmem_map_t memmap)
{
    return memmap & 0xc0; // highest two bits contain flag
}

static bool
has_type(pmem_map_t memmap, enum pmem_type type)
{
    return get_type(memmap) == type;
}

static unsigned long
get_ref(pmem_map_t memmap)
{
    return memmap & 0x3f;
}

static bool
can_ref(pmem_map_t memmap)
{
    return get_ref(memmap) < 0x3f;
}

static bool
can_unref(pmem_map_t memmap)
{
    return !!get_ref(memmap);
}

static pmem_map_t
ref_frame(pmem_map_t memmap)
{
    return memmap + 1;
}

static pmem_map_t
unref_frame(pmem_map_t memmap)
{
    return memmap - 1;
}

static bool
checked_ref_frame(pmem_map_t* memmap)
{
    if (!can_ref(*memmap)) {
        return false;
    }
    *memmap = ref_frame(*memmap);
    return true;
}

static bool
checked_unref_frame(pmem_map_t* memmap)
{
    if (!can_unref(*memmap)) {
        return false;
    }
    --(*memmap);
    return true;
}

static bool
is_allocable(pmem_map_t memmap)
{
    return has_type(memmap, PMEM_TYPE_AVAILABLE) && !get_ref(memmap);
}

//
// PMEM
//

struct pmem {
    struct semaphore  map_sem;
    pmem_map_t*       map;
    const pmem_map_t* map_end;
};

static size_t
memmap_len(const struct pmem* pmem)
{
    return pmem->map_end - pmem->map;
}

static size_t
memmap_size(const struct pmem* pmem)
{
    return memmap_len(pmem) * sizeof(pmem->map[0]);
}

static struct pmem g_pmem;

int
pmem_init(pmem_map_t* memmap, unsigned long nframes)
{
    int res = semaphore_init(&g_pmem.map_sem, 1);
    if (res < 0) {
        return res;
    }

    g_pmem.map = memmap;
    g_pmem.map_end = g_pmem.map + nframes;

    memset(g_pmem.map, 0, memmap_size(&g_pmem));

    return 0;
}

int
pmem_set_type(unsigned long pfindex, unsigned long nframes,
            enum pmem_type type)
{
    if (nframes > memmap_len(&g_pmem)) {
        return -ENODEV;
    } else if (pfindex > memmap_len(&g_pmem) - nframes) {
        return -ENODEV;
    }

    semaphore_enter(&g_pmem.map_sem);

    pmem_map_t* beg = g_pmem.map + pfindex;
    const pmem_map_t* end = beg + nframes;

    while (beg < end) {
        *beg = set_type(*beg, type);
        ++beg;
    }

    semaphore_leave(&g_pmem.map_sem);

    return 0;
}

static pmem_map_t*
find_allocable_frame(pmem_map_t* beg, const pmem_map_t* end)
{
    while (beg < end && !is_allocable(*beg)) {
        ++beg;
    }
    return beg;
}

static pmem_map_t*
find_non_allocable_frame(pmem_map_t* beg, const pmem_map_t* end)
{
    while (beg < end && is_allocable(*beg)) {
        ++beg;
    }
    return beg;
}

static pmem_map_t*
ref_frame_range(pmem_map_t* beg, const pmem_map_t* end)
{
    while (beg < end) {
        *beg = ref_frame(*beg);
        ++beg;
    }
    return beg;
}

static pmem_map_t*
alloc_frame_range(pmem_map_t* beg, const pmem_map_t* end)
{
    pmem_map_t* end2 = find_non_allocable_frame(beg, end);
    if (end2 != end) {
        return end2;
    }
    return ref_frame_range(beg, end);
}

unsigned long
pmem_alloc_frames(unsigned long nframes)
{
    if (nframes > memmap_len(&g_pmem)) {
        return 0; // ENOMEM
    }

    semaphore_enter(&g_pmem.map_sem);

    unsigned long pfindex = 0;
    pmem_map_t* beg = g_pmem.map + 1; // first page frame is never used
    const pmem_map_t* end = g_pmem.map_end - nframes + 1;

    while (beg < end) {

        // find next unused page frame
        beg = find_allocable_frame(beg, end);
        if (beg == end) {
            break; // no unused page frame found, return ENOMEM
        }

        pmem_map_t* range_end = alloc_frame_range(beg, beg + nframes);
        if (nframes == range_end - beg) {
            pfindex = beg - g_pmem.map;
            break; // alloc'ed range of correct size
        }

        beg = range_end + 1;
    }

    semaphore_leave(&g_pmem.map_sem);

    return pfindex;
}

unsigned long
pmem_alloc_frames_at(unsigned long pfindex, unsigned long nframes)
{
    if (nframes > memmap_len(&g_pmem)) {
        return -ENOMEM;
    } else if (pfindex > memmap_len(&g_pmem) - nframes) {
        return -ENOMEM;
    }

    semaphore_enter(&g_pmem.map_sem);

    pmem_map_t* beg = g_pmem.map + pfindex;

    const pmem_map_t* end2 = alloc_frame_range(beg, beg + nframes);
    if (nframes != end2 - beg) {
        goto err_alloc_frame_range;
    }

    semaphore_leave(&g_pmem.map_sem);

    return pfindex;

err_alloc_frame_range:
    semaphore_leave(&g_pmem.map_sem);
    return 0;
}

int
pmem_claim_frames(unsigned long pfindex, unsigned long nframes)
{
    if (nframes > memmap_len(&g_pmem)) {
        return -ENOMEM;
    } else if (pfindex > memmap_len(&g_pmem) - nframes) {
        return -ENOMEM;
    }

    int res = 0;

    pmem_map_t* beg = g_pmem.map + pfindex;
    const pmem_map_t* end = beg + nframes;

    semaphore_enter(&g_pmem.map_sem);

    while (beg < end) {
        if (!checked_ref_frame(beg)) {
            res = -EOVERFLOW;
            goto err_checked_ref_frame;
        }
        ++beg;
    }

    semaphore_leave(&g_pmem.map_sem);

    return 0;

err_checked_ref_frame:
    end = g_pmem.map + pfindex;
    while (beg > end) {
        --beg;
        *beg = unref_frame(*beg);
    }
    semaphore_leave(&g_pmem.map_sem);
    return res;
}

int
pmem_ref_frames(unsigned long pfindex, unsigned long nframes)
{
    if (nframes > memmap_len(&g_pmem)) {
        return -ENOMEM;
    } else if (pfindex > memmap_len(&g_pmem) - nframes) {
        return -ENOMEM;
    }

    int res = 0;

    pmem_map_t* beg = g_pmem.map + pfindex;
    const pmem_map_t* end = beg + nframes;

    semaphore_enter(&g_pmem.map_sem);

    while (beg < end) {
        if (!get_ref(*beg)) {
            res = -ENODEV;
            goto err_get_ref;
        }
        if (!checked_ref_frame(beg)) {
            res = -EOVERFLOW;
            goto err_checked_ref_frame;
        }
        ++beg;
    }

    semaphore_leave(&g_pmem.map_sem);

    return 0;

err_checked_ref_frame:
err_get_ref:
    end = g_pmem.map + pfindex;
    while (beg > end) {
        --beg;
        *beg = unref_frame(*beg);
    }
    semaphore_leave(&g_pmem.map_sem);
    return res;
}

void
pmem_unref_frames(unsigned long pfindex, unsigned long nframes)
{
    if (nframes > memmap_len(&g_pmem)) {
        return;
    } else if (pfindex > memmap_len(&g_pmem) - nframes) {
        return;
    }

    pmem_map_t* beg = g_pmem.map + pfindex;
    const pmem_map_t* end = beg + nframes;

    semaphore_enter(&g_pmem.map_sem);

    while (beg < end) {
        checked_unref_frame(beg); // ignore errors
        ++beg;
    }

    semaphore_leave(&g_pmem.map_sem);
}

const pmem_map_t*
pmem_get_memmap()
{
    return g_pmem.map;
}

size_t
pmem_get_nframes()
{
    return memmap_len(&g_pmem);
}

size_t
pmem_get_size()
{
    return pmem_get_nframes() * PAGEFRAME_SIZE;
}
