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

#include "drivers/crt/crt.h"

struct multiboot_vga_drv {
    struct crt_drv drv;

    unsigned short fb_w;
    unsigned short fb_h;
    unsigned char* vmem;
};

int
multiboot_vga_early_init(struct multiboot_vga_drv* vga,
                         unsigned short fb_w,
                         unsigned short fb_h);

int
multiboot_vga_late_init(struct multiboot_vga_drv* vga);

void
multiboot_vga_uninit(struct multiboot_vga_drv* vga);
