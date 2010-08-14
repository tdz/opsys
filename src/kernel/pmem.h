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

enum
{
        PMEM_FLAG_USEABLE  = 0,    /**< available for use */
        PMEM_FLAG_RESERVED = 1<<0, /**< reserved by system */
        PMEM_ALL_FLAGS     = PMEM_FLAG_USEABLE|
                             PMEM_FLAG_RESERVED
};

int
pmem_init(unsigned long physmap, unsigned long nframes);

int
pmem_set_flags(unsigned long pfindex, unsigned long pfcount,
               unsigned char flags);

unsigned long
pmem_alloc_frames(unsigned long nframes);

unsigned long
pmem_alloc_frames_at(unsigned long pfindex, unsigned long nframes);

int
pmem_ref_frames(unsigned long pfindex, unsigned long nframes);

void
pmem_unref_frames(unsigned long pfindex, unsigned long nframes);

size_t
pmem_get_nframes(void);

size_t
pmem_get_size(void);

