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

struct multiboot_header
{
        unsigned long magic;
        unsigned long flags;
        unsigned long checksum;
        unsigned long header_addr;
        unsigned long load_addr;
        unsigned long load_end_addr;
        unsigned long bss_end_addr;
        unsigned long entry_addr;
};

struct multiboot_entry
{

};

struct elf_section_header_table
{
        unsigned long num;
        unsigned long size;
        unsigned long addr;
        unsigned long shndx;
};

struct multiboot_info
{
        unsigned long flags;
        unsigned long mem_lower;
        unsigned long mem_upper;
        unsigned long boot_device;
        unsigned long cmdline;
        unsigned long mods_count;
        unsigned long mods_addr;
        struct elf_section_header_table elf_sec_hdr;
        unsigned long mmap_length;
        unsigned long mmap_addr;
};

