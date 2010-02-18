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

void
bitset_set(unsigned char *bitset, size_t bit);

void
bitset_unset(unsigned char *bitset, size_t bit);

void
bitset_setto(unsigned char *bitset, size_t bit, int set);

int
bitset_isset(const unsigned char *bitset, size_t bit);

ssize_t
bitset_find_unset(const unsigned char *bitset, size_t len);
