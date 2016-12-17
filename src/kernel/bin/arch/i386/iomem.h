/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2016  Thomas Zimmermann
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

#include <sys/types.h>

struct vmem;

int
init_iomem(struct vmem* vmem);

void
uninit_iomem(void);

void*
map_io_range_nopg(const void* io_addr, const void* virt_addr, size_t length,
                  unsigned int flags);

void*
map_io_range(const void* phys_addr, size_t length, unsigned int flags);

void
unmap_io_range(const void* virt_addr, size_t length);
