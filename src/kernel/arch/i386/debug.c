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

#include "debug.h"

void
db0_setup(unsigned long addr, enum db_length len,
                              enum db_mode mode, int enabled)
{
        static const unsigned int enablebits[] = {
                0x00<<0, /* disable */
                0x03<<0}; /* enable */

        static const unsigned int modebits[] = {
                0x00<<16, /* IFETCH */
                0x01<<16, /* DWRITE */
                0x03<<16}; /* DRDWR */

        static const unsigned int lenbits[] = {
                0x00<<18, /* 1 byte */
                0x01<<18, /* 2 bytes */
                0x03<<18}; /* 4 bytes */

        __asm__(/* disable breakpoint 0 */
                "movl %%dr7, %%eax\n\t"
                "andl $0xfff0fffc, %%eax\n\t"
                "movl %%eax, %%dr7\n\t"
                /* load address */
                "movl %0, %%dr0\n\t"
                /* setup breakpoint 0 */
                "orl %1, %%eax\n\t"
                "orl %2, %%eax\n\t"
                "orl %3, %%eax\n\t"
                "movl %%eax, %%dr7\n\t"
                        :
                        : "r"(addr),
                          "r"(lenbits[len]),
                          "r"(modebits[mode]),
                          "r"(enablebits[!!enabled])
                        : "eax"
                        );
}

