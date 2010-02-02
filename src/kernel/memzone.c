/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2010  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "bitset.h"

/* virtual memory */
#include <page.h>
#include <pte.h>
#include <pagetbl.h>
#include <pde.h>
#include <pagedir.h>
#include <vmemarea.h>
#include "virtmem.h"

#include "memzone.h"

enum {
        MEMZONE_FLAG_ALLOCED = 1<<0,
        MEMZONE_FLAG_INUSE   = 1<<1
};

enum {
        BITS_PER_LARGEPAGE = PAGE_SIZE*1024 * 8,
        BITS_PER_PAGE      = 2
};

static ssize_t
logbase2(unsigned long v)
{
        ssize_t lgv;

        for (lgv = -1; v; v>>=1) {
                ++lgv;
        }

        return lgv;
}

static int
memzone_get_flags(struct memzone *mz, os_index_t i)
{
        return bitset_isset(mz->flags, 2*i) |
              (bitset_isset(mz->flags, 2*i+1)<<1);
}

static void
memzone_set_flags(struct memzone *mz, os_index_t i, int flags)
{
        bitset_setto(mz->flags, 2*i,   flags&(1<<0));
        bitset_setto(mz->flags, 2*i+1, flags&(1<<1));
}

static void
memzone_add_flags(struct memzone *mz, os_index_t i, int flags)
{
        memzone_set_flags(mz, i, memzone_get_flags(mz, i)|flags);
}

static void
memzone_del_flags(struct memzone *mz, os_index_t i, int flags)
{
        memzone_set_flags(mz, i, memzone_get_flags(mz, i)^flags);
}

static os_index_t
memzone_search_unused(struct memzone *mz, size_t nchunks)
{
        int err;
        os_index_t chunk;
        int avail;

        chunk = 0;

        while (chunk < (mz->nchunks-nchunks)) {

                size_t j = 0;

                do {
                        int flags = memzone_get_flags(mz, chunk+j);

                        avail = (flags&MEMZONE_FLAG_ALLOCED) &
                               !(flags&MEMZONE_FLAG_INUSE);
                        ++j;
                } while (avail && (j < nchunks));

                if (!avail) {
                        break;
                }

                chunk += j;
        }

        if (!avail) {
                err = -EAGAIN;
                goto err_avail;
        }

        return chunk;

err_avail:
        return err;
}

static os_index_t
memzone_find_unused(struct memzone *mz, size_t nchunks)
{
        ssize_t err;
        os_index_t chunk;

        if ((chunk = memzone_search_unused(mz, nchunks)) < 0) {
                const struct virtmem_area *area;
                size_t pgcount;
                ssize_t pgindex;
                size_t pgnchunks;
                size_t i;

                if (chunk != -EAGAIN) {
                        err = chunk;
                        goto err_memzone_search_unused;
                }

                pgcount = page_count(0, nchunks*mz->chunksize);

                pgindex = virtmem_alloc_pages_in_area(mz->pd, pgcount,
                                                      mz->areaname,
                                                      PTE_FLAG_PRESENT|
                                                      PTE_FLAG_WRITEABLE);
                if (pgindex < 0) {
                        err = pgindex;
                        goto err_memzone_search_unused;
                }

                area = virtmem_area_get_by_name(mz->areaname);

                chunk = page_memory(pgindex-area->pgindex) / mz->chunksize;

                pgnchunks = pgcount / mz->chunksize;

                for (i = chunk; pgnchunks; --pgnchunks) {
                        memzone_set_flags(mz, i, MEMZONE_FLAG_ALLOCED);
                }
        }

        return chunk;

err_memzone_search_unused:
        return err;
}

#include "console.h"

int
memzone_init(struct memzone *mz,
             struct page_directory *pd, enum virtmem_area_name areaname)
{
        const struct virtmem_area *area;
        int err;
        ssize_t pgindex;
        size_t memsz; /* size of memory */

        area = virtmem_area_get_by_name(areaname);

        memsz = page_memory(area->npages);

        pgindex = virtmem_alloc_pages_in_area(pd,
                                              1024, /* one largepage */
                                              areaname,
                                              PTE_FLAG_PRESENT|
                                              PTE_FLAG_WRITEABLE);
        if (pgindex < 0) {
                err = pgindex;
                goto err_virtmem_alloc_page_in_area;
        }

        mz->flagslen = 1024*PAGE_SIZE;
        mz->flags = page_address(pgindex);
        mz->nchunks = BITS_PER_LARGEPAGE / BITS_PER_PAGE;
        mz->chunksize = 1<<(logbase2(memsz / mz->nchunks)+1);
        mz->nchunks = memsz/mz->chunksize;
        mz->pd = pd;
        mz->areaname = areaname;

        memset(mz->flags, 0, mz->flagslen);

        console_printf("%s:%x mz->chunksize=%x\n", __FILE__, __LINE__, mz->chunksize);

        return 0;

err_virtmem_alloc_page_in_area:
        return err;
}

size_t
memzone_get_nchunks(struct memzone *mz, size_t nbytes)
{
        return nbytes ? 1 + ((nbytes-1) / mz->chunksize) : 0;
}

os_index_t
memzone_alloc(struct memzone *mz, size_t nchunks)
{
        ssize_t err;
        os_index_t chunk;
        ssize_t i;

        /* find allocated, but unused memory zones */

        if ((chunk = memzone_find_unused(mz, nchunks)) < 0) {
                err = chunk;
                goto err_memzone_find_unused;
        }

        /* mark memory zones as used */

        for (i = 0; i < nchunks; ++i) {
                memzone_add_flags(mz, chunk+i, MEMZONE_FLAG_INUSE);
        }

        return chunk;

err_memzone_find_unused:
        return err;
}

void
memzone_free(struct memzone *mz, os_index_t mzoff, size_t nchunks)
{
        ssize_t i;

        /* mark memory zones as unused */

        for (i = 0; i < nchunks; ++i) {
                memzone_del_flags(mz, mzoff+i, MEMZONE_FLAG_INUSE);
        }
}

void *
memzone_address(struct memzone *mz, os_index_t chunk)
{
        const struct virtmem_area *area =
                virtmem_area_get_by_name(mz->areaname);

        return ((unsigned char*)page_address(area->pgindex)) + 
                        chunk*mz->chunksize;
}

