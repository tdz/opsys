/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "types.h"
#include "string.h"

int
memcmp(const void *s1, const void *s2, size_t n)
{
        int res;
        const unsigned char *c1, *c2;

        res = 0;
        c1 = s1;
        c2 = s2;

        while (n && !res) {
                res = *c1 == *c2 ? 0 : *c1 < *c2 ? -1 : 1;
                --n;
                ++s1;
                ++s2;
        }

        return res;
}

void *
memcpy(void *dest, const void *src, size_t n)
{
        unsigned char *cdest;
        const unsigned char *csrc;

        cdest = dest;
        csrc = src;

        while (n) {
                *cdest = *csrc;
                --n;
                ++cdest;
                ++csrc;
        }

        return dest;
}

void *
memset(void *mem, int c, size_t n)
{
        unsigned char *s = mem;

        while (n) {
                *s = c;
                ++s;
                --n;
        }

        return mem;
}

size_t
strlen(const char *str)
{
        size_t len = 0;
        
        while (*str) {
                ++str;
                ++len;
        }

        return len;
}
