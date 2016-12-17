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

#include <stdint.h>
#include <sys/types.h>

enum pmem_type {
    PMEM_TYPE_NONE      = 0,            /**< no memory installed */
    PMEM_TYPE_AVAILABLE = 0x01 << 6,    /**< available for use */
    PMEM_TYPE_SYSTEM    = 0x02 << 6     /**< used by system */
};

/** represents individual entries in the memory map */
typedef uint8_t pmem_map_t;

int
pmem_init(pmem_map_t* memmap, unsigned long nframes);

int
pmem_set_type(unsigned long pfindex, unsigned long pfcount,
              enum pmem_type type);

unsigned long
pmem_alloc_frames(unsigned long nframes);

unsigned long
pmem_alloc_frames_at(unsigned long pfindex, unsigned long nframes);

int
pmem_claim_frames(unsigned long pfindex, unsigned long nframes);

int
pmem_ref_frames(unsigned long pfindex, unsigned long nframes);

void
pmem_unref_frames(unsigned long pfindex, unsigned long nframes);

const pmem_map_t*
pmem_get_memmap(void);

size_t
pmem_get_nframes(void);

size_t
pmem_get_size(void);
