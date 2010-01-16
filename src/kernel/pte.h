/*
 *  oskernel - A small experimental operating-system kernel
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

enum {
        PTE_FLAG_PRESENT   = 1<<0,
        PTE_FLAG_WRITEABLE = 1<<1,
        PTE_FLAG_USERMODE  = 1<<2,
        PTE_ALL_FLAGS      = PTE_FLAG_PRESENT|
                             PTE_FLAG_WRITEABLE|
                             PTE_FLAG_USERMODE
};

enum {
        PTE_STATE_ACCESSED = 1<<5,
        PTE_STATE_DIRTY    = 1<<6
};

typedef unsigned long pte_type;

pte_type
pte_create(unsigned long pfindex, unsigned long flags);

unsigned long
pte_get_pageframe_index(pte_type pte);

