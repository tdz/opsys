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

#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "spinlock.h"
#include "semaphore.h"

/* physical memory */
#include <pageframe.h>

/* virtual memory */
#include <page.h>
#include <pte.h>
#include <vmem.h>

#include "elf.h"
#include "elfldr.h"

/* program headers
 */

static int
elf_loader_construct_phdr_null(const struct vmem *as,
                               const Elf32_Phdr * elf_phdr,
                               const unsigned char *img,
                               struct vmem *dst_as)
{
        return 0;
}

static int
elf_loader_construct_phdr_load(const struct vmem *as,
                               const Elf32_Phdr * elf_phdr,
                               const unsigned char *img,
                               struct vmem *dst_as)
{
        int err;

        err = vmem_alloc_frames(dst_as,
                                   pageframe_index(elf_phdr->p_offset + img),
                                   page_index((void *)elf_phdr->p_vaddr),
                                   page_count((void *)elf_phdr->p_vaddr,
                                                      elf_phdr->p_filesz),
                                   PTE_FLAG_PRESENT|PTE_FLAG_WRITEABLE|
                                   PTE_FLAG_USERMODE);
        if (err < 0)
        {
                goto err_vmem_alloc_pageframes;
        }

        /*
         * set remaining bytes to zero 
         */

        if (elf_phdr->p_filesz < elf_phdr->p_memsz)
        {
                unsigned char *vaddr = (unsigned char *)elf_phdr->p_vaddr;
                memset(vaddr + elf_phdr->p_filesz, 0, elf_phdr->p_memsz -
                       elf_phdr->p_filesz);
        }

        return 0;

err_vmem_alloc_pageframes:
        return err;
}

static int
elf_loader_construct_phdr(const struct vmem *as,
                          const Elf32_Phdr * elf_phdr,
                          const unsigned char *img,
                          struct vmem *dst_as)
{
        static int (*const construct_phdr[]) (const struct vmem *,
                                              const Elf32_Phdr *,
                                              const unsigned char *,
                                              struct vmem *) =
        {
        elf_loader_construct_phdr_null, elf_loader_construct_phdr_load};

        /*
         * some sanity checks 
         */

        if (!
            (elf_phdr->p_type <
             sizeof(construct_phdr) / sizeof(construct_phdr[0]))
            || !construct_phdr[elf_phdr->p_type])
        {
                return 0;
        }

        return construct_phdr[elf_phdr->p_type] (as, elf_phdr, img, dst_as);
}

int
elf_loader_exec(const struct vmem *as,
                const unsigned char *img,
                void **ip, struct vmem *dst_as)
{
        const Elf32_Ehdr *elf_ehdr;
        size_t i;
        int err;

        elf_ehdr = (const Elf32_Ehdr *)img;

        /*
         * some sanity checks 
         */

        if (!elf_loader_is_elf(img) ||
            (elf_ehdr->e_ident[EI_CLASS] != ELFCLASS32) ||
            (elf_ehdr->e_ident[EI_DATA] != ELFDATA2LSB) ||
            (elf_ehdr->e_ident[EI_VERSION] != EV_CURRENT) ||
            (elf_ehdr->e_type != ET_EXEC) ||
            (elf_ehdr->e_machine != EM_386) ||
            (elf_ehdr->e_version != EV_CURRENT) ||
            (!elf_ehdr->e_entry) || (!elf_ehdr->e_phoff))
        {
                err = -ENOEXEC;
                goto err_checks;
        }

        /*
         * construct sections from program headers 
         */

        for (i = 0; i < elf_ehdr->e_phnum; ++i)
        {

                const Elf32_Phdr *elf_phdr;

                elf_phdr = (const Elf32_Phdr *)((img) +
                                                elf_ehdr->e_phoff +
                                                elf_ehdr->e_phentsize * i);

                err = elf_loader_construct_phdr(as, elf_phdr, img, dst_as);

                if (err < 0)
                {
                        goto err_elf_loader_construct_phdr;
                }
        }

        /*
         * init TCB of first thread 
         */

        *ip = (void *)elf_ehdr->e_entry;

        return 0;

err_elf_loader_construct_phdr:
err_checks:
        return err;
}

int
elf_loader_is_elf(const unsigned char *img)
{
        static const char ident[4] = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3 };

        return !memcmp(img, ident, sizeof(ident) / sizeof(ident[0]));
}
