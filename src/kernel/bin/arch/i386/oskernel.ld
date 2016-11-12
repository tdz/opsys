/*
 * opsys - A small, experimental operating system
 * Copyright (C) 2016  Thomas Zimmermann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

ENTRY(_start)

SECTIONS {
    /* The kernel is loaded at 1 MiB */
    . = 0x100000;

    .text : {
        /* The section .multiboot goes first. It contains the Multiboot
         * header, which contains the entry point for the boot loader's
         * call into the kernel. It has to be located in the first 8 KiB
         * of the binary. */
        *(.multiboot)
        /* Now comes the actual .text section with the instructions. */
        *(.text)
    }
    /* Multiboot requires that the .data section comes after .text. */
    .data : {
        *(.data)
        _edata = .; /* end of text + data; requried by Multiboot  */
    }
    /* The .bss section is 0'ed out by the boot loader, so we put it in
     * at the end. */
    .bss  : { *(.bss) }
    _end = .;
}
