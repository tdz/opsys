/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009  Thomas Zimmermann
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

#include "multiboot_vga.h"
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include "iomem.h"
#include "ioports.h"
#include "pde.h"

static struct multiboot_vga_drv*
multiboot_vga_drv_of_crt_drv(struct crt_drv* drv)
{
    return containerof(drv, struct multiboot_vga_drv, drv);
}

static void*
get_addr(const struct multiboot_vga_drv* mb_vga, unsigned long off)
{
    return mb_vga->vmem + 2 * off;
}

static int
get_fb_resolution(struct crt_drv* drv,
                  unsigned short* r, unsigned short* c)
{
    const struct multiboot_vga_drv* mb_vga = multiboot_vga_drv_of_crt_drv(drv);

    *r = mb_vga->fb_h;
    *c = mb_vga->fb_w;

    return 0;
}

static long
get_fb_offset(struct crt_drv* drv,
              unsigned short r, unsigned short c)
{
    const struct multiboot_vga_drv* mb_vga = multiboot_vga_drv_of_crt_drv(drv);

    return c + r * mb_vga->fb_w;
}

static int
set_cursor_offset(struct crt_drv* drv, unsigned long off)
{
    const struct multiboot_vga_drv* mb_vga = multiboot_vga_drv_of_crt_drv(drv);

    off %= mb_vga->fb_w * mb_vga->fb_h;

    uint8_t cr0e = (off >> 8) & 0xff;
    uint8_t cr0f = off & 0xff;

    io_outb_index(0x03d4, 0x0e, 0x03d5, cr0e);
    io_outb_index(0x03d4, 0x0f, 0x03d5, cr0f);

    return 0;
}

static ssize_t
get_cursor_offset(struct crt_drv* drv)
{
    uint8_t cr0e = io_inb_index(0x3d4, 0x0e, 0x03d5);
    uint8_t cr0f = io_inb_index(0x3d4, 0x0f, 0x03d5);

    uint16_t offset = (cr0e << 8) | cr0f;

    return offset;
}

static int
put_LF(struct crt_drv* drv)
{
    const struct multiboot_vga_drv* mb_vga = multiboot_vga_drv_of_crt_drv(drv);

    ssize_t res = get_cursor_offset(drv);
    if (res < 0) {
        return res;
    }
    return set_cursor_offset(drv, res + mb_vga->fb_w);
}

static int
put_CR(struct crt_drv* drv)
{
    const struct multiboot_vga_drv* mb_vga = multiboot_vga_drv_of_crt_drv(drv);

    ssize_t res = get_cursor_offset(drv);
    if (res < 0) {
        return res;
    }
    return set_cursor_offset(drv, res - (res % mb_vga->fb_w));
}

static int
put_char(struct crt_drv* drv, unsigned long off, int c)
{
    const struct multiboot_vga_drv* mb_vga = multiboot_vga_drv_of_crt_drv(drv);

    off %= mb_vga->fb_w * mb_vga->fb_h;

    unsigned char* vmem = get_addr(mb_vga, off);

    if (!vmem) {
        return -EFAULT;
    }

    vmem[0] = c;
    vmem[1] = 0x07;

    return 1;
}

int
multiboot_vga_early_init(struct multiboot_vga_drv* mb_vga,
                         unsigned short fb_w,
                         unsigned short fb_h)
{
    static const struct crt_drv_funcs funcs = {
        get_fb_resolution,
        get_fb_offset,
        set_cursor_offset,
        get_cursor_offset,
        put_CR,
        put_LF,
        put_char,
    };

    assert(mb_vga);

    int res = crt_drv_init(&mb_vga->drv, &funcs);
    if (res < 0) {
        return res;
    }

    mb_vga->fb_w = fb_w;
    mb_vga->fb_h = fb_h;
    mb_vga->vmem = (unsigned char*)0xb8000;

    return 0;
}

int
multiboot_vga_late_init(struct multiboot_vga_drv* mb_vga)
{
    void* mem = map_io_range_nopg(mb_vga->vmem, mb_vga->vmem, 64 * 1024,
                                  PDE_FLAG_PRESENT | PDE_FLAG_WRITEABLE);
    if (!mem) {
        return -ENOMEM;
    }

    mb_vga->vmem = mem;

    return 0;
}

void
multiboot_vga_uninit(struct multiboot_vga_drv* mb_vga)
{
    assert(mb_vga);

    crt_drv_uninit(&mb_vga->drv);
}
