/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2010  Thomas Zimmermann <tdz@users.sourceforge.net>
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

enum virtmem_area_name;
struct address_space;

struct memzone
{
        size_t                 flagslen;
        unsigned char         *flags;
        size_t                 chunksize;
        size_t                 nchunks;
        struct address_space  *as;
        enum virtmem_area_name areaname;
};

int
memzone_init(struct memzone *mz,
             struct address_space *as, enum virtmem_area_name areaname);

size_t
memzone_get_nchunks(struct memzone *mz, size_t nbytes);

os_index_t
memzone_alloc(struct memzone *mz, size_t nchunks);

void
memzone_free(struct memzone *mz, os_index_t chunk, size_t nchunks);

void *
memzone_address(struct memzone *mz, os_index_t chunk);

