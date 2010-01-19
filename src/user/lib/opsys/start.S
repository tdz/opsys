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

.text

.global _start
.global exit

_start:
        pushl $0 /* TODO: push argv */
        pushl $0 /* TODO: push argc */
        call main
        pushl %eax /* push exit status */
exit:
        /* when we jump here, the exit status should be on the stack already */
        pushl $0 /* push exit system-call number */
        int $0x80 /* this never returns */

/* Temporary workaround, as opsys does not yet quit tasks */
do_hlt: 
        hlt
        jmp do_hlt

