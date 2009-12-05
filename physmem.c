/*
 *  oskernel - A small experimental operating-system kernel
 *  Copyright (C) 2009  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "types.h"
#include "string.h"
#include "physpage.h"
#include "physmem.h"

/* memory-map offset at 24 MiB */
static unsigned char *g_physmap = (unsigned char*)0x01800000;
static unsigned long  g_physmap_npages = 0;

int
physmem_init(unsigned long npages)
{
        memset(g_physmap, 0, npages*sizeof(g_physmap[0]));
        g_physmap_npages = npages;

        /* add global variables of physmem */
        physmem_add_area(((unsigned long)g_physmap)>>PHYSPAGE_SHIFT, 1,
                         PHYSMEM_FLAG_RESERVED);
        physmem_add_area(g_physmap_npages>>PHYSPAGE_SHIFT, 1,
                         PHYSMEM_FLAG_RESERVED);

        /* add physmap */
        physmem_add_area(((unsigned long)g_physmap>>PHYSPAGE_SHIFT),
                         g_physmap_npages+1,
                         PHYSMEM_FLAG_RESERVED);

        return 0;
}

int
physmem_add_area(unsigned long pgoffset,
                 unsigned long npages,
                 unsigned char flags)
{
        unsigned char *physmap = g_physmap+pgoffset;

        while (npages--) {
                *(physmap++) |= (flags&0x1)<<7;
        }

        return 0;
}

unsigned long
physmem_alloc(unsigned long npages)
{
        unsigned long pgoffset;
        unsigned char *beg;
        const unsigned char *end;

        /* FIXME: lock here */

        pgoffset = 0;
        beg = g_physmap+1; /* first page not used */
        end = g_physmap+g_physmap_npages-npages;

        while (!pgoffset && (beg < end)) {

                /* find next useable page */
                for (; *beg && (beg < end); ++beg) {}

                /* end reached */
                if (beg == end) {
                        break;
                }

                /* empty page found */
                {
                        unsigned char *beg2;
                        const unsigned char *end2;

                        beg2 = beg;
                        end2 = beg+npages;

                        /* check empty block */
                        for (; (beg2 < end2) && !(*beg2); ++beg2) {}

                        /* block not empty */
                        if (beg2 < end2) {
                                break;
                        }

                        /* empty block found*/
                        for (beg2 = beg; beg2 < end2; ++beg2) {
                                *beg2 = 1 | (PHYSMEM_FLAG_RESERVED<<7);
                        }
                        pgoffset = beg-g_physmap;
                }
        }

        /* FIXME: unlock here */
        return pgoffset;
}

int
physmem_ref(unsigned pgoffset, unsigned long npages)
{
        unsigned long i;
        unsigned char *physmap;

        /* FIXME: lock here */

        physmap = g_physmap+pgoffset;

        /* check for allocation and max refcount */
        for (i = 0; i < npages; ++i) {
                if (((*physmap) == 0xff) || !(*physmap)) {
                        return -1;
                }
        }

        physmap = g_physmap+pgoffset;

        /* increment refcount */
        for (i = 0; i < npages; ++i) {
                *physmap = (PHYSMEM_FLAG_RESERVED<<7) + ((*physmap)&0x7f) + 1;
                ++physmap;
        }

        /* FIXME: unlock here */
        return 0;
}

void
physmem_unref(unsigned pgoffset, unsigned long npages)
{
        unsigned char *physmap;

        /* FIXME: lock here */

        physmap = g_physmap+pgoffset;

        while (npages--) {
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

