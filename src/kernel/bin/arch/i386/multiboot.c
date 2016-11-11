/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009-2010  Thomas Zimmermann
 *  Copyright (C) 2016       Thomas Zimmermann
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

#include "multiboot.h"
#include <errno.h>
#include <stdint.h>
#include "console.h"
#include "cpu.h"
#include "loader.h"
#include "main.h"
#include "pageframe.h"
#include "pmem.h"
#include "sched.h"
#include "taskhlp.h"
#include "tcbhlp.h"

static void *
mmap_base_addr(const struct multiboot_mmap *mmap)
{
        /* only lowest 4 byte are available in 32-bit protected mode */
        return (void *)(intptr_t) ((((uint64_t) mmap->base_addr_high) << 32)
                                   + mmap->base_addr_low);
}

static uint64_t
mmap_length(const struct multiboot_mmap *mmap)
{
        return (((uint64_t) mmap->length_high) << 32) + mmap->length_low;
}

static int
range_order(unsigned long beg1, unsigned long end1,
            unsigned long beg2, unsigned long end2)
{
        if (end1 <= beg2)
        {
                return -1;
        }
        else if (end2 <= beg1)
        {
                return 1;
        }

        return 0;
}

static unsigned long
find_unused_area(const struct multiboot_header *mb_header,
                 const struct multiboot_info *mb_info, unsigned long nframes)
{
        size_t i;
        unsigned long mmap_addr;
        unsigned long kernel_pfindex, kernel_nframes;
        unsigned long pfoffset;

        kernel_pfindex = pageframe_index((void *)mb_header->load_addr);
        kernel_nframes = pageframe_span((void *)mb_header->load_addr,
                                        mb_header->bss_end_addr -
                                        mb_header->load_addr + 1);

        mmap_addr = mb_info->mmap_addr;

        for (pfoffset = 0, i = 0; !pfoffset && (i < mb_info->mmap_length);)
        {
                const struct multiboot_mmap *mmap;
                unsigned long area_pfindex;
                unsigned long area_nframes;
                unsigned long pfindex;

                mmap = (const struct multiboot_mmap *)mmap_addr;

                /* next entry address  */

                mmap_addr += mmap->size + 4;
                i += mmap->size + 4;

                /* area is not useable */
                if (mmap->type != 1)
                {
                        continue;
                }

                /* area page index and length */

                area_pfindex = pageframe_index(mmap_base_addr(mmap));
                area_nframes = pageframe_count(mmap_length(mmap));

                /* area at address 0, or too small */

                if (!area_pfindex || (area_nframes < nframes))
                {
                        continue;
                }

                /* possible index */

                pfindex = area_pfindex;

                /* check for intersection with kernel */

                if (!range_order(kernel_pfindex,
                                 kernel_pfindex + kernel_nframes,
                                 pfindex, pfindex + nframes))
                {
                        pfindex = kernel_pfindex + kernel_nframes;
                }

                /* check for intersection with modules */

                while (!pfoffset &&
                       ((pfindex + nframes) < (area_pfindex + area_nframes)))
                {

                        size_t j;
                        const struct multiboot_module *mod;

                        mod = (const struct multiboot_module *)mb_info->
                                mods_addr;

                        for (j = 0; j < mb_info->mods_count; ++j, ++mod)
                        {
                                unsigned long mod_pfindex;
                                unsigned long mod_nframes;

                                mod_pfindex =
                                        pageframe_index((void *)mod->
                                                        mod_start);
                                mod_nframes =
                                        pageframe_count(mod->mod_end -
                                                        mod->mod_start);

                                /* check intersection */

                                if (range_order(mod_pfindex,
                                                mod_pfindex + mod_nframes,
                                                pfindex, pfindex + nframes))
                                {
                                        /* no intersection, offset found */
                                        pfoffset = pageframe_offset(pfindex);
                                }
                                else
                                {
                                        pfindex = mod_pfindex + mod_nframes;
                                }
                        }
                }
        }

        return pfoffset;
}

static int
multiboot_init_pmem(const struct multiboot_header *mb_header,
                       const struct multiboot_info *mb_info)
{
        int err;
        unsigned long pfindex, pfcount;

        if (!(mb_info->flags & MULTIBOOT_INFO_FLAG_MEM))
        {
                err = -EINVAL;
                goto err_multiboot_info_flag_mem;
        }

        pfcount = pageframe_span(0, (mb_info->mem_upper + 1024) << 10);

        pfindex = find_unused_area(mb_header,
                                   mb_info, pfcount >> PAGEFRAME_SHIFT);

        console_printf("found physmap area at %x\n", (unsigned long)pfindex);

        return pmem_init(pfindex, pfcount);

err_multiboot_info_flag_mem:
        return err;
}

static int
multiboot_init_pmem_kernel(const struct multiboot_header *mb_header)
{
        return pmem_set_flags(pageframe_index
                              ((void *)mb_header->load_addr),
                              pageframe_count(mb_header->bss_end_addr -
                                              mb_header->load_addr),
                              PMEM_FLAG_RESERVED);
}

static int
multiboot_init_pmem_mmap(const struct multiboot_info *mb_info)
{
        size_t i;
        unsigned long mmap_addr;

        if (!(mb_info->flags & MULTIBOOT_INFO_FLAG_MMAP))
        {
                return 0;
        }

        mmap_addr = mb_info->mmap_addr;

        for (i = 0; i < mb_info->mmap_length;)
        {
                const struct multiboot_mmap *mmap;
                unsigned long pfindex;
                unsigned long nframes;

                mmap = (const struct multiboot_mmap *)mmap_addr;

                pfindex = pageframe_index(mmap_base_addr(mmap));
                nframes = pageframe_count(mmap_length(mmap));

                console_printf("pfindex=%x nframes=%x type=%x\n",pfindex, nframes, mmap->type);

                pmem_set_flags(pfindex,
                               nframes,
                               mmap->type == 1 ? PMEM_FLAG_USEABLE
                                               : PMEM_FLAG_RESERVED);

                mmap_addr += mmap->size + 4;
                i += mmap->size + 4;
        }

        return 0;
}

static int
multiboot_init_pmem_module(const struct multiboot_module *mod)
{
        return pmem_set_flags(pageframe_index((void *)mod->mod_start),
                              pageframe_count(mod->mod_end -
                                              mod->mod_start),
                              PMEM_FLAG_RESERVED);
}

static int
multiboot_init_pmem_modules(const struct multiboot_info *mb_info)
{
        int err;
        const struct multiboot_module *mod, *modend;

        if (!(mb_info->flags & MULTIBOOT_INFO_FLAG_MODS))
        {
                return 0;
        }

        err = 0;
        mod = (const struct multiboot_module *)mb_info->mods_addr;
        modend = mod + mb_info->mods_count;

        while ((mod < modend)
               && ((err = multiboot_init_pmem_module(mod)) < 0))
        {
                ++mod;
        }

        return err;
}

static int
multiboot_load_modules(struct task *parent,
                       const struct multiboot_info *mb_info)
{
        int err;
        const struct multiboot_module *mod;
        size_t i;

        if (!(mb_info->flags & MULTIBOOT_INFO_FLAG_MODS))
        {
                return 0;
        }

        mod = (const struct multiboot_module *)mb_info->mods_addr;

        for (i = 0; i < mb_info->mods_count; ++i, ++mod)
        {
                struct task *tsk;
                struct tcb *tcb;
                void *ip;

                if (mod->string)
                {
                        console_printf("loading module '\%s'\n", mod->string);
                }
                else
                {
                        console_printf("loading module %x\n", i);
                }

                /*
                 * allocate task
                 */

                err = task_helper_allocate_task_from_parent(parent, &tsk);

                if (err < 0)
                {
                        console_perror
                                ("task_helper_allocate_task_from_parent",
                                 -err);
                        goto err_task_helper_allocate_task_from_parent;
                }

                /*
                 * allocate tcb
                 */

                err = tcb_helper_allocate_tcb_and_stack(tsk, 1, &tcb);

                if (err < 0)
                {
                        console_perror("tcb_helper_allocate_tcb_and_stack",
                                       -err);
                        goto err_tcb_helper_allocate_tcb_and_stack;
                }

                /*
                 * load binary image
                 */

                if ((err =
                     loader_exec(tcb, (void *)mod->mod_start, &ip, tcb)) < 0)
                {
                        console_perror("loader_exec", -err);
                        goto err_loader_exec;
                }

                /*
                 * set thread to starting state
                 */
                err = tcb_helper_run_user_thread(sched_get_current_thread(cpuid()),
                                                 tcb, ip);

                if (err < 0)
                {
                        goto err_tcb_helper_run_user_thread;
                }

                /*
                 * schedule thread
                 */
                if ((err = sched_add_thread(tcb, 64)) < 0)
                {
                        console_perror("sched_add_thread", -err);
                        goto err_sched_add_thread;
                }

                console_printf("scheduled as %x.\n", err);

                continue;

        err_sched_add_thread:
        err_tcb_helper_run_user_thread:
        err_loader_exec:
                /*
                 * FIXME: free tcb
                 */
        err_tcb_helper_allocate_tcb_and_stack:
                /*
                 * FIXME: free task
                 */
        err_task_helper_allocate_task_from_parent:
                continue;
        }

        return 0;
}

void
multiboot_init(const struct multiboot_header *mb_header,
               const struct multiboot_info *mb_info, void *stack)
{
        int err;
        struct task *tsk;

        console_printf("opsys booting...\n");

        /* init physical memory with lowest 4 MiB reserved for DMA,
         * kernel, etc
         */

        if ((err = multiboot_init_pmem(mb_header, mb_info)) < 0)
        {
                console_perror("multiboot_init_pmem", -err);
                return;
        }

        pmem_set_flags(0, 1024, PMEM_FLAG_RESERVED);

        if ((err = multiboot_init_pmem_kernel(mb_header)) < 0)
        {
                console_perror("multiboot_init_pmem_kernel", -err);
                return;
        }
        if ((err = multiboot_init_pmem_mmap(mb_info)) < 0)
        {
                console_perror("multiboot_init_pmem_mmap", -err);
                return;
        }
        if ((err = multiboot_init_pmem_modules(mb_info)) < 0)
        {
                console_perror("multiboot_init_pmem_modules", -err);
                return;
        }

        /* setup virtual memory and system services
         */

        if ((err = general_init(&tsk, &stack)) < 0)
        {
                console_perror("general_init", -err);
                return;
        }

        /* load modules as ELF binaries
         */

        if ((err = multiboot_load_modules(tsk, mb_info)) < 0)
        {
                console_perror("multiboot_load_modules", -err);
                return;
        }
}

