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

#pragma once

enum
{
        PDE_FLAG_PRESENT   = 1<<0,
        PDE_FLAG_WRITEABLE = 1<<1,
        PDE_FLAG_USERMODE  = 1<<2,
        PDE_FLAG_WRTHROUGH = 1<<3,
        PDE_FLAG_CACHED    = 1<<4,
        PDE_FLAG_LARGEPAGE = 1<<7,
        PDE_ALL_FLAGS      = PDE_FLAG_PRESENT|
                             PDE_FLAG_WRITEABLE|
                             PDE_FLAG_USERMODE|
                             PDE_FLAG_WRTHROUGH|
                             PDE_FLAG_CACHED|
                             PDE_FLAG_LARGEPAGE
};

enum
{
        PDE_STATE_ACCESSED = 1<<5
};

typedef unsigned long pde_type;

pde_type
pde_create(unsigned long pfindex, unsigned long flags);

unsigned long
pde_get_pageframe_index(pde_type pde);
