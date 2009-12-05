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
static unsigned char *physmap = (unsigned char*)0x01800000;
static unsigned long  physmap_npages = 0;

int
physmem_init(unsigned long npages)
{
        memset(physmap, 0, npages*sizeof(physmap[0]));
        physmap_npages = npages;

        /* add global variables of physmem */
        physmem_add_area(((unsigned long)physmap)>>PHYSPAGE_SHIFT, 1,
                         PHYSMEM_FLAG_RESERVED);
        physmem_add_area(((unsigned long)physmap_npages)>>PHYSPAGE_SHIFT, 1,
                         PHYSMEM_FLAG_RESERVED);

        /* add physmap */
        physmem_add_area(((unsigned long)physmap>>PHYSPAGE_SHIFT),
                         physmap_npages+1,
                         PHYSMEM_FLAG_RESERVED);

        return 0;
}

int
physmem_add_area(unsigned long pgoffset,
                 unsigned long npages,
                 unsigned char flags)
{
        unsigned char *physmap = physmap+pgoffset;

        while (npages--) {
                *(physmap++) |= (flags&0x3)<<7;
        }

        return 0;
}

