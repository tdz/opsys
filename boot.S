/*
 *  oskernel - A small experimental operating-system kernel
 *  Copyright (C) 2009  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#define MULTIBOOT_HEADER_MAGIC  0x1BADB001
#define MULTIBOOT_HEADER_FLAGS  0x00000003

.text

.global _start

_start:
        jmp multiboot_entry

        .align 4

multiboot_header:

        .long 0x1BADB002 # MULTIBOOT_HEADER_MAGIC
        .long 0x00000003 # MULTIBOOT_HEADER_FLAGS
        .long -(0x1BADB002+0x00000003)
        .long multiboot_header
        .long _start
        .long _edata
        .long _end
        .long multiboot_entry

multiboot_entry:

	movl $(stack+4096), %esp

	pushl $0
	popf

        # Call C main function

	pushl %ebx
	call os_main_from_multiboot

#	hlt

        int $0x1

	hlt

        # print

        movl $4, %eax  # (Syscall 4: write)
        movl $1, %ebx  # (stdout)
        movl $HW, %ecx # "Hello world!"
        movl $13, %edx # strlen("Hello world!")
        int $0x80

        # Exit

        movl $1, %eax
        movl $0, %ebx
        int $0x80

.data

HW: .ascii "Hello world!\n"

.comm stack,4096

