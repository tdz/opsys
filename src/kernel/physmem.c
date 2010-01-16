/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009-2010  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "minmax.h"
#include "stddef.h"
#include "types.h"
#include "string.h"
#include "pageframe.h"
#include "physmem.h"

static unsigned char *g_physmap = NULL;
static unsigned long  g_physmap_nframes = 0;

static int
physmem_add_self(void)
{
        /* add global variables of physmem */
        physmem_add_area(pageframe_index((unsigned long)&g_physmap),
                         pageframe_count(sizeof(g_physmap)),
                         PHYSMEM_FLAG_RESERVED);
        physmem_add_area(pageframe_index((unsigned long)&g_physmap_nframes),
                         pageframe_count(sizeof(g_physmap_nframes)),
                         PHYSMEM_FLAG_RESERVED);

        /* add physmap */
        physmem_add_area(pageframe_index((unsigned long)g_physmap),
                         pageframe_count(g_physmap_nframes*sizeof(g_physmap[0])),
                         PHYSMEM_FLAG_RESERVED);

        return 0;
}

int
physmem_init(unsigned long physmap, unsigned long nframes)
{
        g_physmap = (unsigned char*)physmap;

        memset(g_physmap, 0, nframes*sizeof(g_physmap[0]));
        g_physmap_nframes = nframes;

        return physmem_add_self();
}

int
physmem_add_area(unsigned long pfindex,
                 unsigned long nframes,
                 unsigned char flags)
{
        unsigned char *beg;
        const unsigned char *end;

        /* FIXME: lock here */

        beg = g_physmap+pfindex;
        end = beg+nframes;

        while ((beg < end) && (beg < g_physmap+g_physmap_nframes)) {
                *beg |= (flags&PHYSMEM_ALL_FLAGS)<<7;
                ++beg;
        }

        /* FIXME: unlock here */

        return 0;
}

unsigned long
physmem_alloc_frames(unsigned long nframes)
{
        unsigned long pfindex;
        unsigned char *beg;
        const unsigned char *end;

        /* FIXME: lock here */

        pfindex = 0;
        beg = g_physmap+1; /* first page not used */
        end = g_physmap+g_physmap_nframes-nframes;

        while (!pfindex && (beg < end)) {

                /* find next useable page */
                for (; (beg < end) && (*beg); ++beg) {}

                /* end reached */
                if (beg == end) {
                        break;
                }

                /* empty page found */
                {
                        unsigned char *beg2;
                        const unsigned char *end2;

                        beg2 = beg;
                        end2 = beg+nframes;

                        /* check empty block */
                        for (; (beg2 < end2) && !(*beg2); ++beg2) {}

                        /* block not empty */
                        if (beg2 < end2) {
                                beg = beg2+1; /* next possible empty block */
                                continue;
                        }

                        /* empty block found*/
                        for (beg2 = beg; beg2 < end2; ++beg2) {
                                *beg2 = (PHYSMEM_FLAG_RESERVED<<7) + 1;
                        }

                        pfindex = beg-g_physmap;
                }
        }

        /* FIXME: unlock here */

        return pfindex;
}

unsigned long
physmem_alloc_frames_at(unsigned long pfindex, unsigned long nframes)
{
        unsigned char *beg;
        const unsigned char *end;

        /* FIXME: lock here */

        beg = g_physmap+pfindex;
        end = g_physmap+nframes;

        /* find next useable page */
        for (; (beg < end) && !(*beg); ++beg) {}

        /* stopped too early, pages already allocated */
        if (beg < end) {
                return 0;
        }

        /* range is empty */

        beg = g_physmap+pfindex;

        for (beg = g_physmap+pfindex; beg < end; ++beg) {
                *beg = (PHYSMEM_FLAG_RESERVED<<7) + 1;
        }

        /* FIXME: unlock here */

        return pfindex;
}

int
physmem_ref_frames(unsigned long pfindex, unsigned long nframes)
{
        unsigned long i;
        unsigned char *physmap;

        /* FIXME: lock here */

        physmap = g_physmap+pfindex;

        /* check for allocation and max refcount */
        for (i = 0; i < nframes; ++i) {
                if (((*physmap) == 0xff) || !(*physmap)) {
                        return -1;
                }
        }

        physmap = g_physmap+pfindex;

        /* increment refcount */
        for (i = 0; i < nframes; ++i) {
                *physmap = (PHYSMEM_FLAG_RESERVED<<7) + ((*physmap)&0x7f) + 1;
                ++physmap;
        }

        /* FIXME: unlock here */
        return 0;
}

void
physmem_unref_frames(unsigned long pfindex, unsigned long nframes)
{
        unsigned char *physmap;

        /* FIXME: lock here */

        physmap = g_physmap+pfindex;

        while (nframes--) {
                if ((*physmap) == ((PHYSMEM_FLAG_RESERVED<<7)|1)) {
                        *physmap = PHYSMEM_FLAG_USEABLE<<7;
                } else {
                        *physmap = (PHYSMEM_FLAG_RESERVED<<7) +
                                   ((*physmap)&0x7f) - 1;
                }
                ++physmap;
        }

        /* FIXME: unlock here */
}

