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
        SYSCALL_TASK_QUIT = 0,
        SYSCALL_CRT_WRITE,
        SYSCALL_CRT_GETSIZE,
        SYSCALL_CRT_GETPOS,
        SYSCALL_CRT_SETPOS
};

int
syscall_task_quit(void);

int
syscall_crt_write(const char *buf, size_t buflen, unsigned char attr);

int
syscall_crt_getmaxpos(unsigned short *row, unsigned short *col);

int
syscall_crt_getpos(unsigned short *row, unsigned short *col);

int
syscall_crt_setpos(unsigned short row, unsigned short col);

