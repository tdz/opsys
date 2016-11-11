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

#include <sys/types.h>

/*
 * \brief computes the number of elements in an array
 */
#define ARRAY_NELEMS(x)  \
        ( sizeof(x)/sizeof((x)[0]) )

int
memcmp(const void *s1, const void *s2, size_t n);

void *
memcpy(void *dest, const void *src, size_t n);

void *
memset(void *mem, int c, size_t n);

char *
strerror(int errnum);

char *
strerror_l(int errnum, char *strerrbuf, size_t buflen);

size_t
strlen(const char *str);
