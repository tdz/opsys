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

#include "vga.h"
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include "ioports.h"

enum {
    MAX_ROW = 25,
    MAX_COL = 80
};

static void*
get_addr(struct crt_drv* drv, unsigned long off)
{
    return ((unsigned char *)0xb8000) + 2 * off;
}

static int
get_fb_resolution(struct crt_drv* drv,
                  unsigned short* r, unsigned short* c)
{
    *r = MAX_ROW;
    *c = MAX_COL;

    return 0;
}

static long
get_fb_offset(struct crt_drv* drv,
              unsigned short r, unsigned short c)
{
    return c + r * MAX_COL;
}

static int
set_cursor_offset(struct crt_drv* drv, unsigned long off)
{
    off %= MAX_ROW * MAX_COL;

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
    ssize_t res = get_cursor_offset(drv);
    if (res < 0) {
        return res;
    }
    return set_cursor_offset(drv, res + MAX_COL);
}

static int
put_CR(struct crt_drv* drv)
{
    ssize_t res = get_cursor_offset(drv);
    if (res < 0) {
        return res;
    }
    return set_cursor_offset(drv, res - (res % MAX_COL));
}

static int
put_char(struct crt_drv* drv, unsigned long off, int c)
{
    off %= MAX_ROW * MAX_COL;

    unsigned char* vmem = get_addr(drv, off);

    if (!vmem) {
        return -EFAULT;
    }

    vmem[0] = c;
    vmem[1] = 0x07;

    return 1;
}

int
vga_init(struct vga_drv* vga)
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

    assert(vga);

    int res = crt_drv_init(&vga->drv, &funcs);
    if (res < 0) {
        return res;
    }

    return 0;
}

void
vga_uninit(struct vga_drv* vga)
{
    assert(vga);

    crt_drv_uninit(&vga->drv);
}
