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

#include <elf.h>
#include "types.h"
#include "elfldr.h"

int
elf_load_section(const Elf32_Shdr *elf_shdr, const void *elfimg)
{
        return 0;
}

int
elf_load_phdr(const Elf32_Phdr *elf_phdr, const void *elfimg)
{
        if (elf_phdr->p_type == PT_NULL) {
                return 0;
        }
        if (elf_phdr->p_type != PT_LOAD) {
                return -1;
        }



        return 0;
}

int
elf_load(const void *elfimg)
{
        static const char ident[4] = {EI_MAG0, EI_MAG1, EI_MAG2, EI_MAG3};

        const Elf32_Ehdr *elf_ehdr;
        size_t i;

        elf_ehdr = elfimg;

        /* Some sanity checks */

        if (memcmp(elf_ehdr->e_ident, ident, 4) ||
            (elf_ehdr->e_ident[EI_CLASS] != ELFCLASS32) ||
            (elf_ehdr->e_ident[EI_DATA] != ELFDATA2LSB) ||
            (elf_ehdr->e_ident[EI_VERSION] != EV_CURRENT) ||
            (elf_ehdr->e_type != ET_EXEC) ||
            (elf_ehdr->e_machine != EM_386) ||
            (elf_ehdr->e_version != EV_CURRENT) ||
            (!elf_ehdr->e_entry) ||
            (!elf_ehdr->e_phoff)) {
                return -1;
        }

        for (i = 0; i < elf_ehdr->e_phnum; ++i) {

                const Elf32_Phdr *elf_phdr;
                int res;

                elf_phdr = (const Elf32_Phdr*)(((const unsigned char*)elfimg) +
                        elf_ehdr->e_phoff +
                        elf_ehdr->e_phentsize*i);

                if ((res = elf_load_phdr(elf_phdr, elfimg)) < 0) {
                        return res;
                }
        }

        return 0;
}

