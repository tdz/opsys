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

#include "stddef.h"
#include "types.h"
#include "string.h"
#include "page.h"
#include "physmem.h"

static unsigned char *g_physmap = NULL;
static unsigned long  g_physmap_npages = 0;

int
physmem_init(unsigned int physmap, unsigned long npages)
{
        g_physmap = (unsigned char*)physmap;

        memset(g_physmap, 0, npages*sizeof(g_physmap[0]));
        g_physmap_npages = npages;

        /* add global variables of physmem */
        physmem_add_area(((unsigned long)g_physmap)>>PAGE_SHIFT, 1,
                         PHYSMEM_FLAG_RESERVED);
        physmem_add_area(g_physmap_npages>>PAGE_SHIFT, 1,
                         PHYSMEM_FLAG_RESERVED);

        /* add physmap */
        physmem_add_area(((unsigned long)g_physmap>>PAGE_SHIFT),
                         g_physmap_npages+1,
                         PHYSMEM_FLAG_RESERVED);

        return 0;
}

int
physmem_add_area(unsigned long pgindex,
                 unsigned long npages,
                 unsigned char flags)
{
        unsigned char *physmap;

        /* FIXME: lock here */

        physmap = g_physmap+pgindex;

        while (npages-- && (physmap < (g_physmap+g_physmap_npages))) {
                *(physmap++) |= (flags&0x1)<<7;
        }

        /* FIXME: unlock here */

        return 0;
}

unsigned long
physmem_alloc_pages(unsigned long npages)
{
        unsigned long pgindex;
        unsigned char *beg;
        const unsigned char *end;

        /* FIXME: lock here */

        pgindex = 0;
        beg = g_physmap+1; /* first page not used */
        end = g_physmap+g_physmap_npages-npages;

        while (!pgindex && (beg < end)) {

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
                                beg = beg2+1; /* next possible empty block */
                                continue;
                        }

                        /* empty block found*/
                        for (beg2 = beg; beg2 < end2; ++beg2) {
                                *beg2 = (PHYSMEM_FLAG_RESERVED<<7) + 1;
                        }
                        pgindex = beg-g_physmap;
                }
        }

        /* FIXME: unlock here */

        return pgindex;
}

unsigned long
physmem_alloc_pages_at(unsigned long pgindex, unsigned long npages)
{
        unsigned char *beg;
        const unsigned char *end;

        /* FIXME: lock here */

        beg = g_physmap+pgindex;
        end = g_physmap+npages;

        /* find next useable page */
        for (; (beg < end) && !(*beg); ++beg) {}

        /* stopped too early, pages already allocated */
        if (beg < end) {
                return 0;
        }

        /* range is empty */

        beg = g_physmap+pgindex;

        for (beg = g_physmap+pgindex; beg < end; ++beg) {
                *beg = (PHYSMEM_FLAG_RESERVED<<7) + 1;
        }

        /* FIXME: unlock here */

        return pgindex;
}

int
physmem_ref_pages(unsigned long pgindex, unsigned long npages)
{
        unsigned long i;
        unsigned char *physmap;

        /* FIXME: lock here */

        physmap = g_physmap+pgindex;

        /* check for allocation and max refcount */
        for (i = 0; i < npages; ++i) {
                if (((*physmap) == 0xff) || !(*physmap)) {
                        return -1;
                }
        }

        physmap = g_physmap+pgindex;

        /* increment refcount */
        for (i = 0; i < npages; ++i) {
                *physmap = (PHYSMEM_FLAG_RESERVED<<7) + ((*physmap)&0x7f) + 1;
                ++physmap;
        }

        /* FIXME: unlock here */
        return 0;
}

void
physmem_unref_pages(unsigned long pgindex, unsigned long npages)
{
        unsigned char *physmap;

        /* FIXME: lock here */

        physmap = g_physmap+pgindex;

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

