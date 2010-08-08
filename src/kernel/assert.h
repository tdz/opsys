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

#if NDEBUG != 0
        #define assert(c_)

        #define assert_never_reach()
#else
        #define assert(c_)                                                      \
                {                                                               \
                        int failed = !(c_);                                     \
                        if (failed)                                             \
                        {                                                       \
                                __assert_failed(#c_, __FILE__, __LINE__);       \
                        }                                                       \
                }

        #define assert_never_reach()                                            \
                {                                                               \
                        __assert_failed("never reach", __FILE__, __LINE__);     \
                }
#endif

void
__assert_failed(const char *cond, const char *file, unsigned long line);

