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

unsigned long
pmem_alloc_frames(unsigned long nframes)
{
        unsigned long pfindex;
        pmem_map_t* beg;
        const pmem_map_t* end;

        if (nframes > memmap_len(&g_pmem)) {
            return 0; // ENOMEM
        }

        semaphore_enter(&g_pmem.map_sem);

        pfindex = 0;
        beg = g_pmem.map + 1;   /* first page frame is never used */
        end = g_pmem.map_end - nframes;

        while (!pfindex && (beg < end))
        {
                /*
                 * find next useable page
                 */
                for (; (beg < end) && !is_allocable(*beg); ++beg)
                {
                }

                /*
                 * end reached
                 */

                if (beg == end)
                {
                        break;
                }

                /*
                 * empty page found
                 */
                {
                        pmem_map_t *beg2;
                        const pmem_map_t *end2;

                        beg2 = beg;
                        end2 = beg + nframes;

                        /*
                         * check empty block
                         */
                        for (; (beg2 < end2) && is_allocable(*beg2); ++beg2)
                        {
                        }

                        /*
                         * block not empty
                         */
                        if (beg2 < end2)
                        {
                                beg = beg2 + 1; /* next possible empty block */
                                continue;
                        }

                        /*
                         * reference page frames
                         */
                        for (beg2 = beg; beg2 < end2; ++beg2)
                        {
                                *beg2 = ref_frame(*beg2);
                        }

                        pfindex = beg - g_pmem.map;
                }
        }

        semaphore_leave(&g_pmem.map_sem);

        return pfindex;
}

unsigned long
pmem_alloc_frames_at(unsigned long pfindex, unsigned long nframes)
{
        pmem_map_t *beg;
        const pmem_map_t *end;

        semaphore_enter(&g_pmem.map_sem);

        beg = g_pmem.map + pfindex;
        end = g_pmem.map + nframes;

        /*
         * find next useable page
         */
        for (; (beg < end) && is_allocable(*beg); ++beg)
        {
        }

        /*
         * stopped too early, pages already allocated
         */
        if (beg < end)
        {
                goto err_beg_lt_end;
        }

        /*
         * reference page frames
         */

        beg = g_pmem.map + pfindex;

        for (beg = g_pmem.map + pfindex; beg < end; ++beg)
        {
                *beg = ref_frame(*beg);
        }

        semaphore_leave(&g_pmem.map_sem);

        return pfindex;

err_beg_lt_end:
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

    /* increment refcount */

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
        unsigned long i;
        pmem_map_t *physmap;

        semaphore_enter(&g_pmem.map_sem);

        physmap = g_pmem.map + pfindex;

        /*
         * check for allocation and max refcount
         */
        for (i = 0; i < nframes; ++i)
        {
                if (!can_ref(*physmap))
                {
                        semaphore_leave(&g_pmem.map_sem);
                        return -1;
                }
        }

        physmap = g_pmem.map + pfindex;

        /*
         * increment refcount
         */
        for (i = 0; i < nframes; ++i)
        {
                *physmap = ref_frame(*physmap);
                ++physmap;
        }

        semaphore_leave(&g_pmem.map_sem);

        return 0;
}

void
pmem_unref_frames(unsigned long pfindex, unsigned long nframes)
{
        pmem_map_t *physmap;

        semaphore_enter(&g_pmem.map_sem);

        physmap = g_pmem.map + pfindex;

        while (nframes--)
        {
                checked_unref_frame(physmap); // ignore errors
                ++physmap;
        }

        semaphore_leave(&g_pmem.map_sem);
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

