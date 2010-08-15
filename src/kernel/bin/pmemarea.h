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

/*
 * \brief names the available areas of physical memory
 */
enum pmem_area_name
{
        PMEM_AREA_DMA_20 = 0, /**< DMA-capable area <1 MiB */
        PMEM_AREA_DMA_24,     /**< DMA-capable area <16 MiB */
        PMEM_AREA_DMA_32,     /**< DMA-capable area <4 GiB */
        PMEM_AREA_DMA    = PMEM_AREA_DMA_32, /**< any DMA-capable area */
        PMEM_AREA_USER, /**< memory available for allocation */
        LAST_PMEM_AREA /**< the index of the last entry in pmem_area_name */
};

struct pmem_area
{
        os_index_t   pfindex;
        size_t       nframes;
};

const struct pmem_area *
pmem_area_get_by_name(enum pmem_area_name name);

const struct pmem_area *
pmem_area_get_by_frame(os_index_t pgindex);

int
pmem_area_contains_frame(const struct pmem_area *area, os_index_t pfindex);

