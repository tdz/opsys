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

#ifndef PHYSMEM_H
#define PHYSMEM_H

enum {
        PHYSMEM_FLAG_USEABLE  = 0, /* available for use */
        PHYSMEM_FLAG_RESERVED = 1<<0 /* reserved by system */
};

int
physmem_init(unsigned long npages);

int
physmem_add_area(unsigned long pgoffset,
                 unsigned long npages,
                 unsigned char flags);

unsigned long
physmem_alloc(unsigned long npages);

int
physmem_ref(unsigned pgoffset, unsigned long npages);

void
physmem_unref(unsigned pgoffset, unsigned long npages);

#endif

