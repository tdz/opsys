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

struct crt_drv;

struct crt_drv_funcs {
    int (*get_fb_resolution)(struct crt_drv*, unsigned short*, unsigned short*);
    long (*get_fb_offset)(struct crt_drv*, unsigned short, unsigned short);
    int (*set_cursor_offset)(struct crt_drv*, unsigned long);
    ssize_t (*get_cursor_offset)(struct crt_drv*);
    int (*put_CR)(struct crt_drv*);
    int (*put_LF)(struct crt_drv*);
    int (*put_char)(struct crt_drv*, unsigned long, int c);
};

struct crt_drv {
    const struct crt_drv_funcs* funcs;
};

int
crt_drv_init(struct crt_drv* drv, const struct crt_drv_funcs* funcs);

void
crt_drv_uninit(struct crt_drv* drv);

static inline int
crt_drv_get_fb_resolution(struct crt_drv* drv,
                          unsigned short* r, unsigned short* c)
{
    return drv->funcs->get_fb_resolution(drv, r, c);
}

static inline ssize_t
crt_drv_get_fb_offset(struct crt_drv* drv,
                      unsigned short r, unsigned short c)
{
    return drv->funcs->get_fb_offset(drv, r, c);
}

static inline int
crt_drv_set_cursor_offset(struct crt_drv* drv, unsigned long off)
{
    return drv->funcs->set_cursor_offset(drv, off);
}

static inline ssize_t
crt_drv_get_cursor_offset(struct crt_drv* drv)
{
    return drv->funcs->get_cursor_offset(drv);
}

static inline int
crt_drv_put_LF(struct crt_drv* drv)
{
    return drv->funcs->put_LF(drv);
}

static inline int
crt_drv_put_CR(struct crt_drv* drv)
{
    return drv->funcs->put_CR(drv);
}

static inline int
crt_drv_put_char(struct crt_drv* drv, unsigned long off, int c)
{
    return drv->funcs->put_char(drv, off, c);
}

static inline ssize_t
crt_drv_put_str(struct crt_drv* drv, unsigned long off,
                const char* buf, size_t buflen)
{
    ssize_t len = 0;

    while (buflen) {
        int res = drv->funcs->put_char(drv, off + len, *buf);
        if (res < 0) {
            return res;
        }
        ++len;
        ++buf;
        --buflen;
    }

    return len;
}
