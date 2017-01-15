/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2010  Thomas Zimmermann
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

#define va_list                 void*

#define va_start(__ap, __arg)   ((__ap) = ((char*)&(__arg))+sizeof(__arg))

#define va_arg(__ap, __type)    (*((__type*)(__ap)));   \
                                {char *cp = (__ap);     \
                                 cp += sizeof(__type);  \
                                 (__ap) = cp;}

#define va_end(__ap)
