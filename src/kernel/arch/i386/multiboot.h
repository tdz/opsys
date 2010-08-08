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
        MULTIBOOT_HEADER_MAGIC = 0x1badb002
};

enum {
        MULTIBOOT_HEADER_FLAG_ALIGNED = 1<<0,
        MULTIBOOT_HEADER_FLAG_MEMINFO = 1<<1,
        MULTIBOOT_HEADER_FLAG_VIDMODE = 1<<2,
        MULTIBOOT_HEADER_FLAG_BINAOUT = 1<<16
};

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
        unsigned long mode_type;
        unsigned long width;
        unsigned long height;
        unsigned long depth;
};

struct aout_symbol_table
{
        unsigned long tabsize;
        unsigned long strsize;
        unsigned long addr;
        unsigned long reserved;
};

struct elf_section_header_table
{
        unsigned long num;
        unsigned long size;
        unsigned long addr;
        unsigned long shndx;
};

enum {
        MULTIBOOT_INFO_FLAG_MEM        = 1<<0,
        MULTIBOOT_INFO_FLAG_BOOTDEV    = 1<<1,
        MULTIBOOT_INFO_FLAG_CMDLINE    = 1<<2,
        MULTIBOOT_INFO_FLAG_MODS       = 1<<3,
        MULTIBOOT_INFO_FLAG_AOUTSYMS   = 1<<4,
        MULTIBOOT_INFO_FLAG_ELFSYMS    = 1<<5,
        MULTIBOOT_INFO_FLAG_MMAP       = 1<<6,
        MULTIBOOT_INFO_FLAG_DRIVES     = 1<<7,
        MULTIBOOT_INFO_FLAG_CONFIG     = 1<<8,
        MULTIBOOT_INFO_FLAG_BOOTLOADER = 1<<9,
        MULTIBOOT_INFO_FLAG_APM        = 1<<10,
        MULTIBOOT_INFO_FLAG_VBE        = 1<<11
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
        union {
                struct aout_symbol_table        aout_sym_tab;
                struct elf_section_header_table elf_sec;
        } syms;
        unsigned long mmap_length;
        unsigned long mmap_addr;
};

struct multiboot_module
{
        unsigned long  mod_start;
        unsigned long  mod_end;
        unsigned char *string;
        unsigned long  reserved;
};

struct multiboot_mmap
{
        unsigned long size;
        unsigned long base_addr_low;
        unsigned long base_addr_high;
        unsigned long length_low;
        unsigned long length_high;
        unsigned long type;
};

void
multiboot_main(const struct multiboot_header *mb_header,
               const struct multiboot_info *mb_info,
               void *stack);

