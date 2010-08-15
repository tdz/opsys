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

/**
 * \brief a spinlock
 */
typedef volatile unsigned long  spinlock_type;

/**
 * \brief initializes a spinlock
 * \param[in] spinlock the spinlock
 * \return 0 on success, or a negative error code otherwise
 */
int
spinlock_init(spinlock_type *spinlock);

/**
 * \brief cleans up spinlock
 * \param[in] spinlock the spinlock
 */
void
spinlock_uninit(spinlock_type *spinlock);

/**
 * \brief attempts to grab a spinlock
 * \param[in] spinlock the spinlock
 * \param uid a unique id to write into the spinlock
 * \return 0 on success, or a negative error code otherwise
 */
int
spinlock_try_lock(spinlock_type *spinlock, unsigned long uid);

/**
 * \brief grabs a spinlock
 * \param[in] spinlock the spinlock
 * \param uid a unique id to write into the spinlock
 *
 * \attention This function busy-waits for the spinlock to become empty and
 *            can block for indefinite amounts of time.
 */
void
spinlock_lock(spinlock_type *spinlock, unsigned long uid);

/**
 * \brief releases a spinlock
 * \param[in] spinlock the spinlock
 */
void
spinlock_unlock(spinlock_type *spinlock);

