/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2010       Thomas Zimmermann
 *  Copyright (C) 2016-2017  Thomas Zimmermann
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

#include "bitset.h"
#include <errno.h>

void
bitset_set(unsigned char *bitset, unsigned long bit)
{
        bitset[bit >> 3] |= 1 << (bit & 0x07);
}

void
bitset_unset(unsigned char *bitset, unsigned long bit)
{
        bitset[bit >> 3] &= ~(1 << (bit & 0x07));
}

void
bitset_setto(unsigned char *bitset, unsigned long bit, int set)
{
    static void (*const setto[]) (unsigned char *, unsigned long) = {
        bitset_set,
        bitset_unset
    };
    setto[!set] (bitset, bit);
}

int
bitset_isset(const unsigned char *bitset, unsigned long bit)
{
        return !!(bitset[bit >> 3] & (1 << (bit & 0x07)));
}

ssize_t
bitset_find_unset(const unsigned char *bitset, size_t len)
{
        size_t i;
        unsigned long bit;

        bit = 0;

        for (i = 0; i < len; ++i)
        {
                ssize_t j;
                for (j = 0; j < 8; ++j, ++bit)
                {
                        if (!bitset_isset(bitset, bit))
                        {
                                return bit;
                        }
                }
        }

        return -EAGAIN;
}
