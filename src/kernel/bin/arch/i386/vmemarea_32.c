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

#include "vmemarea_32.h"

/* VMEM areas for x86 32-bit Protected Mode.
 */
const struct vmem_area g_vmem_area[LAST_VMEM_AREA] =
{
    /* < 4 KiB */
    [VMEM_AREA_SYSTEM]     = { .pgindex = 0, .npages = 1,
                               .flags = VMEM_AREA_FLAG_EMPTY},
    /* < 4 MiB */
    [VMEM_AREA_KERNEL_LOW] = { .pgindex = 1, .npages = 1023,
                               .flags = VMEM_AREA_FLAG_KERNEL |
                                        VMEM_AREA_FLAG_PAGETABLES |
                                        VMEM_AREA_FLAG_GLOBAL},
    /* < 1 GiB */
    [VMEM_AREA_KERNEL]     = { .pgindex = 1024, .npages = 260096,
                               .flags = VMEM_AREA_FLAG_KERNEL |
                                        VMEM_AREA_FLAG_PAGETABLES |
                                        VMEM_AREA_FLAG_GLOBAL},
    /* < 1 GiB */
    [VMEM_AREA_TMPMAP]     = { .pgindex = 261120, .npages = 1024,
                               .flags = VMEM_AREA_FLAG_KERNEL |
                                        VMEM_AREA_FLAG_PAGETABLES |
                                        VMEM_AREA_FLAG_GLOBAL},
    /* >= 1 GiB */
    [VMEM_AREA_USER]       = { .pgindex = 262144, .npages = 786432,
                               .flags = VMEM_AREA_FLAG_USER}
};
