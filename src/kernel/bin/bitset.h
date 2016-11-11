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

#include <sys/types.h>

/**
 * \brief set a bit in a bitset
 * \param[in,out] bitset the bitset
 * \param bit the bit to set
 */
void
bitset_set(unsigned char *bitset, unsigned long bit);

/**
 * \brief clear a bit in a bitset
 * \param[in,out] bitset the bitset
 * \param bit the bit to set
 */
void
bitset_unset(unsigned char *bitset, unsigned long bit);

/**
 * \brief set a bit in a bitset to specified value
 * \param[in,out] bitset the bitset
 * \param bit the bit to set
 * \param set zero to clear the bit, or any other value to set the bit
 */
void
bitset_setto(unsigned char *bitset, unsigned long bit, int set);

/**
 * \brief check if a bit in a bitset is set
 * \param[in,out] bitset the bitset
 * \param bit the bit to set
 * \return true if the bit is set, or false otherwise
 */
int
bitset_isset(const unsigned char *bitset, unsigned long bit);

/**
 * \brief find first unset bit in a bitset
 * \param[in,out] bitset the bitset
 * \param len the bitset's length
 * \return the unset bit's position, or a negative error code otherwise
 */
ssize_t
bitset_find_unset(const unsigned char *bitset, size_t len);
