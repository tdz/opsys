/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2010  Thomas Zimmermann
 *  Copyright (C) 2016  Thomas Zimmermann
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

#pragma once

typedef unsigned long   Elf32_Addr;
typedef unsigned short  Elf32_Half;
typedef unsigned long   Elf32_Off;
typedef long            Elf32_Sword;
typedef unsigned long   Elf32_Word;

/*
 * ELF header
 */

/* object file type */

#define ET_NONE         0
#define ET_REL          1
#define ET_EXEC         2
#define ET_DYN          3
#define ET_CORE         4
#define ET_LOPROC       0xff00
#define ET_HIPROC       0xffff

/* machine architecture */

#define EM_M32          1
#define EM_SPARC        2
#define EM_386          3
#define EM_68K          4
#define EM_88K          5
#define EM_860          7
#define EM_MIPS         8
#define EM_MIPS_RS4_BE  10

/* version */

#define EV_NONE         0
#define EV_CURRENT      1

/* identification indices */

#define EI_MAG0         0
#define EI_MAG1         1
#define EI_MAG2         2
#define EI_MAG3         3
#define EI_CLASS        4
#define EI_DATA         5
#define EI_VERSION      6
#define EI_PAD          7
#define EI_NIDENT       16

#define ELFMAG0         0x7f
#define ELFMAG1         'E'
#define ELFMAG2         'L'
#define ELFMAG3         'F'

#define ELFCLASSNONE    0
#define ELFCLASS32      1
#define ELFCLASS64      2

#define ELFDATANONE     0
#define ELFDATA2LSB     1
#define ELFDATA2MSB     2

typedef struct {
          unsigned char e_ident[EI_NIDENT];
          Elf32_Half    e_type;
          Elf32_Half    e_machine;
          Elf32_Word    e_version;
          Elf32_Addr    e_entry;
          Elf32_Off     e_phoff;
          Elf32_Off     e_shoff;
          Elf32_Word    e_flags;
          Elf32_Half    e_ehsize;
          Elf32_Half    e_phentsize;
          Elf32_Half    e_phnum;
          Elf32_Half    e_shentsize;
          Elf32_Half    e_shnum;
          Elf32_Half    e_shstrndx;
} Elf32_Ehdr;

/*
 * ELF program header
 */

typedef struct {
          Elf32_Word p_type;
          Elf32_Off  p_offset;
          Elf32_Addr p_vaddr;
          Elf32_Addr p_paddr;
          Elf32_Word p_filesz;
          Elf32_Word p_memsz;
          Elf32_Word p_flags;
          Elf32_Word p_align;
} Elf32_Phdr;

/* segment type */

#define PT_NULL         0
#define PT_LOAD         1
#define PT_DYNAMIC      2
#define PT_INTERP       3
#define PT_NOTE         4
#define PT_SHLIB        5
#define PT_PHDR         6
#define PT_LOPROC       0x70000000
#define PT_HIPROC       0x7fffffff
