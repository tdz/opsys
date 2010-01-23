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

_start:
        jmp multiboot_entry

        .align 4

multiboot_header:
        .long 0x1badb002 /* MULTIBOOT_HEADER_MAGIC */
        .long 0x00000007 /* MULTIBOOT_HEADER_FLAG_ALIGNED|
                            MULTIBOOT_HEADER_FLAG_MEMINFO */
        .long -(0x1badb002+0x00000007)
        /* address fields */
        .long multiboot_header
        .long _start
        .long _edata
        .long _end
        .long multiboot_entry
        /* graphics fields */
        .long 1
        .long 80
        .long 43
        .long 0

multiboot_entry:
        movl $(stack+4096), %esp

        /* reset eflags */
        pushl $0
        popf

        /* call C main function for multiboot */

        pushl %esp
        pushl %ebx
        pushl $multiboot_header
        call multiboot_main

hltloop:
        hlt
        jmp hltloop

.data

.comm stack,4096

