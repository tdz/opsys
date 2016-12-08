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

static struct semaphore g_physmap_sem;
static unsigned char *g_physmap = NULL;
static unsigned long g_physmap_nframes = 0;

static int
pmem_set_flags_self(void)
{
    /* claim physmap; global variables of pmem claimed by kernel image */

    return pmem_claim_frames(pageframe_index(g_physmap),
                             pageframe_count(g_physmap_nframes *
                                             sizeof(g_physmap[0])));
}

int
pmem_init(unsigned long physmap, unsigned long nframes)
{
        int err;

        if ((err = semaphore_init(&g_physmap_sem, 1)) < 0)
        {
                goto err_semaphore_init;
        }

        g_physmap = (unsigned char *)physmap;

        memset(g_physmap, 0, nframes * sizeof(g_physmap[0]));
        g_physmap_nframes = nframes;

        if ((err = pmem_set_flags_self()) < 0)
        {
                goto err_pmem_set_flags_self;
        }

        return 0;

err_pmem_set_flags_self:
        semaphore_uninit(&g_physmap_sem);
err_semaphore_init:
        return err;
}

int
pmem_set_flags(unsigned long pfindex, unsigned long nframes,
               unsigned char flags)
{
        unsigned char *beg;
        const unsigned char *end;

        semaphore_enter(&g_physmap_sem);

        beg = g_physmap + pfindex;
        end = beg + nframes;

        while ((beg < end) && (beg < g_physmap + g_physmap_nframes))
        {
                *beg |= (flags & PMEM_ALL_FLAGS) << 7;
                ++beg;
        }

        semaphore_leave(&g_physmap_sem);

        return 0;
}

unsigned long
pmem_alloc_frames(unsigned long nframes)
{
        unsigned long pfindex;
        unsigned char *beg;
        const unsigned char *end;

        semaphore_enter(&g_physmap_sem);

        pfindex = 0;
        beg = g_physmap + 1;    /* first page not used */
        end = g_physmap + g_physmap_nframes - nframes;

        while (!pfindex && (beg < end))
        {
                /*
                 * find next useable page
                 */
                for (; (beg < end) && (*beg); ++beg)
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
                        unsigned char *beg2;
                        const unsigned char *end2;

                        beg2 = beg;
                        end2 = beg + nframes;

                        /*
                         * check empty block
                         */
                        for (; (beg2 < end2) && !(*beg2); ++beg2)
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
                                *beg2 = (PMEM_FLAG_RESERVED << 7) + 1;
                        }

                        pfindex = beg - g_physmap;
                }
        }

        semaphore_leave(&g_physmap_sem);

        return pfindex;
}

unsigned long
pmem_alloc_frames_at(unsigned long pfindex, unsigned long nframes)
{
        unsigned char *beg;
        const unsigned char *end;

        semaphore_enter(&g_physmap_sem);

        beg = g_physmap + pfindex;
        end = g_physmap + nframes;

        /*
         * find next useable page
         */
        for (; (beg < end) && !(*beg); ++beg)
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

        beg = g_physmap + pfindex;

        for (beg = g_physmap + pfindex; beg < end; ++beg)
        {
                *beg = (PMEM_FLAG_RESERVED << 7) + 1;
        }

        semaphore_leave(&g_physmap_sem);

        return pfindex;

err_beg_lt_end:
        semaphore_leave(&g_physmap_sem);
        return 0;
}

int
pmem_claim_frames(unsigned long pfindex, unsigned long nframes)
{
    if (nframes > g_physmap_nframes) {
        return -ENOMEM;
    } else if (g_physmap_nframes - nframes < pfindex) {
        return -ENOMEM;
    }

    int res = 0;

    unsigned char* beg = g_physmap + pfindex;
    const unsigned char* end = beg + nframes;

    /* increment refcount */

    semaphore_enter(&g_physmap_sem);

    while (beg < end) {
        if (*beg == 0x3f) {
            res = -EOVERFLOW;
            goto err_max_incr;
        }

        *beg = (PMEM_FLAG_RESERVED << 7) | ((*beg & 0x7f) + 1);
        ++(*beg) ;
        ++beg;
    }

    semaphore_leave(&g_physmap_sem);

    return 0;

err_max_incr:
    end = g_physmap + pfindex;
    while (beg > end) {
        --beg;
        --(*beg);
    }
    semaphore_leave(&g_physmap_sem);
    return res;
}

int
pmem_ref_frames(unsigned long pfindex, unsigned long nframes)
{
        unsigned long i;
        unsigned char *physmap;

        semaphore_enter(&g_physmap_sem);

        physmap = g_physmap + pfindex;

        /*
         * check for allocation and max refcount
         */
        for (i = 0; i < nframes; ++i)
        {
                if (((*physmap) == 0xff) || !(*physmap))
                {
                        semaphore_leave(&g_physmap_sem);
                        return -1;
                }
        }

        physmap = g_physmap + pfindex;

        /*
         * increment refcount
         */
        for (i = 0; i < nframes; ++i)
        {
                *physmap = (PMEM_FLAG_RESERVED << 7) + ((*physmap) & 0x7f) + 1;
                ++physmap;
        }

        semaphore_leave(&g_physmap_sem);

        return 0;
}

void
pmem_unref_frames(unsigned long pfindex, unsigned long nframes)
{
        unsigned char *physmap;

        semaphore_enter(&g_physmap_sem);

        physmap = g_physmap + pfindex;

        while (nframes--)
        {
                if ((*physmap) == ((PMEM_FLAG_RESERVED << 7) | 1))
                {
                        *physmap = PMEM_FLAG_USEABLE << 7;
                }
                else
                {
                        *physmap = (PMEM_FLAG_RESERVED << 7) +
                                ((*physmap) & 0x7f) - 1;
                }
                ++physmap;
        }

        semaphore_leave(&g_physmap_sem);
}

size_t
pmem_get_nframes()
{
        return g_physmap_nframes;
}

size_t
pmem_get_size()
{
        return pmem_get_nframes() * PAGEFRAME_SIZE;
}

