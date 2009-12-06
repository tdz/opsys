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
#include "string.h"
#include "console.h"

/* section headers
 */

static int
elf_construct_shdr_null(const Elf32_Shdr *elf_shdr, const void *elfimg)
{
        return 0;
}

static int
elf_construct_shdr_progbits(const Elf32_Shdr *elf_shdr, const void *elfimg)
{
        void *dst;
        const void *src;

        dst = (void*)elf_shdr->sh_addr;
        src = (const void*)(((const unsigned char*)elfimg)+elf_shdr->sh_offset);

        console_printf("src=%x dst=%x\n", (unsigned long)src, (unsigned long)dst);

        if (src && dst && elf_shdr->sh_size) {
                memcpy(dst, src, elf_shdr->sh_size);
        }

        return 0;
}

static int
elf_construct_shdr_symtab(const Elf32_Shdr *elf_shdr, const void *elfimg)
{
        /* section ignored */
        return 0;
}

static int
elf_construct_shdr_strtab(const Elf32_Shdr *elf_shdr, const void *elfimg)
{
        /* section ignored */
        return 0;
}

static int
elf_construct_shdr_rela(const Elf32_Shdr *elf_shdr, const void *elfimg)
{
        /* section ignored */
        return 0;
}

static int
elf_construct_shdr_hash(const Elf32_Shdr *elf_shdr, const void *elfimg)
{
        /* section ignored */
        return 0;
}

static int
elf_construct_shdr_dynamic(const Elf32_Shdr *elf_shdr, const void *elfimg)
{
        /* section ignored */
        return 0;
}

static int
elf_construct_shdr_note(const Elf32_Shdr *elf_shdr, const void *elfimg)
{
        /* section ignored */
        return 0;
}

static int
elf_construct_shdr_nobits(const Elf32_Shdr *elf_shdr, const void *elfimg)
{
        memset((void*)elf_shdr->sh_addr, 0, elf_shdr->sh_size);

        return 0;
}

static int
elf_construct_shdr(const Elf32_Shdr *elf_shdr, const void *elfimg)
{
        static int (* const construct_shdr[])(const Elf32_Shdr*, const void*) = {
                elf_construct_shdr_null,
                elf_construct_shdr_progbits,
                elf_construct_shdr_symtab,
                elf_construct_shdr_strtab,
                elf_construct_shdr_rela,
                elf_construct_shdr_hash,
                elf_construct_shdr_dynamic,
                elf_construct_shdr_note,
                elf_construct_shdr_nobits};

        static size_t construct_shdr_len = sizeof(construct_shdr) /
                                           sizeof(construct_shdr[0]);

/*        console_printf("%s:%x\n", __FILE__, __LINE__);*/

        /* some sanity checks */

        if (!(elf_shdr->sh_type < construct_shdr_len) ||
            !construct_shdr[elf_shdr->sh_type]) {
                return -1;
        }

/*        console_printf("%s:%x\n", __FILE__, __LINE__);*/

        return construct_shdr[elf_shdr->sh_type](elf_shdr, elfimg);
}

/* program headers
 */

static int
elf_construct_phdr_null(const Elf32_Phdr *elf_phdr, const void *elfimg)
{
        return 0;
}

static int
elf_construct_phdr_load(const Elf32_Phdr *elf_phdr, const void *elfimg)
{
        /* copy section content */

        memcpy((void*)elf_phdr->p_vaddr,
               ((const unsigned char*)elfimg)+elf_phdr->p_offset,
               elf_phdr->p_filesz);

        /* set remaining bytes to zero */

        if (elf_phdr->p_filesz < elf_phdr->p_memsz) {
                memset(((unsigned char*)elf_phdr->p_vaddr)+elf_phdr->p_filesz,
                       0,
                       elf_phdr->p_memsz-elf_phdr->p_filesz);
        }

        return 0;
}

static int
elf_construct_phdr(const Elf32_Phdr *elf_phdr, const void *elfimg)
{
        static int (* const construct_phdr[])(const Elf32_Phdr*, const void*) = {
                elf_construct_phdr_null,
                elf_construct_phdr_load};

        /* some sanity checks */

        if (!(elf_phdr->p_type < sizeof(construct_phdr)/sizeof(construct_phdr[0])) ||
            !construct_phdr[elf_phdr->p_type]) {
                return 0;
        }

        return construct_phdr[elf_phdr->p_type](elf_phdr, elfimg);
}

int
elf_exec(const void *elfimg)
{
        static const char ident[4] = {EI_MAG0, EI_MAG1, EI_MAG2, EI_MAG3};

        const Elf32_Ehdr *elf_ehdr;
        size_t i;

        elf_ehdr = elfimg;

        /* Some sanity checks */

        if (!memcmp(elf_ehdr->e_ident, ident, 4) ||
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

        /* construct sections from section headers */

/*        for (i = 0; i < elf_ehdr->e_shnum; ++i) {

                const Elf32_Shdr *elf_shdr;
                int res;

                elf_shdr = (const Elf32_Shdr*)(((const unsigned char*)elfimg) +
                        elf_ehdr->e_shoff +
                        elf_ehdr->e_shentsize*i);

                if ((res = elf_construct_shdr(elf_shdr, elfimg)) < 0) {
                        return res;
                }
        }*/

        /* construct sections from program headers */

        for (i = 0; i < elf_ehdr->e_phnum; ++i) {

                const Elf32_Phdr *elf_phdr;
                int res;

                elf_phdr = (const Elf32_Phdr*)(((const unsigned char*)elfimg) +
                        elf_ehdr->e_phoff +
                        elf_ehdr->e_phentsize*i);

                if ((res = elf_construct_phdr(elf_phdr, elfimg)) < 0) {
                        return res;
                }
        }

        console_printf("%s:%x entry point=%x\n", __FILE__, __LINE__, elf_ehdr->e_entry);

        __asm__("       call *%0\n\t"
                        :
                        : "r"(elf_ehdr->e_entry) );

        return 0;
}

