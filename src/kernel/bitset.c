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

#include <errno.h>
#include <types.h>
#include "bitset.h"

void
bitset_set(unsigned char *bitset, size_t bit)
{
        bitset[bit>>3] |= ~(1<<(bit&0x07));
}

void
bitset_unset(unsigned char *bitset, size_t bit)
{
        bitset[bit>>3] &= ~(1<<(bit&0x07));
}

int
bitset_isset(const unsigned char *bitset, size_t bit)
{
        return !!(bitset[bit>>3] & ~(1<<(bit&0x07)));
}

ssize_t
bitset_find_unset(const unsigned char *bitset, size_t len)
{
        ssize_t i, bit;
        int found;

        bit = 0;
        found = 0;

        for (i = 0; !found && (i < len); ++i) {
                ssize_t j;
                for (j = 0; !found && (j < 8); ++j, ++bit) {
                        found = (bitset[i] ^ (1<<j));
                        ++j;
                }
        }

        if (!found) {
                return -EAGAIN;
        }

        return bit;
}

