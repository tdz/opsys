/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009-2010  Thomas Zimmermann <tdz@users.sourceforge.net>
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

enum {
        IPC_OPSYS_TASK_QUIT = 0,
};

int
syscall(unsigned long r0,
        unsigned long r1,
        unsigned long r2,
        unsigned long r3,
        unsigned long *res0,
        unsigned long *res1,
        unsigned long *res2,
        unsigned long *res3);

int
syscall0(unsigned long r0,
         unsigned long r1,
         unsigned long r2,
         unsigned long r3);

