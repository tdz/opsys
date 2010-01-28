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
#include <stddef.h>
#include <string.h>
#include <types.h>

#include "bitset.h"

/* virtual memory */
#include <page.h>
#include <pte.h>
#include <pagetbl.h>
#include <pde.h>
#include <pagedir.h>
#include <vmemarea.h>
#include <virtmem.h>

#include <alloc.h>

enum {
        MEMZONE_FLAG_ALLOCED = 1<<0,
        MEMZONE_FLAG_INUSE   = 1<<1
};

static size_t         g_memzone_flagslen;
static unsigned char *g_memzone_flags;
static size_t         g_memzone_size;
static size_t         g_nmemzones;

enum {
        BITS_PER_LARGEPAGE = PAGE_SIZE*1024 * 8,
        BITS_PER_PAGE      = 2
};

static int
memzone_get_flags(os_index_t i)
{
        return bitset_isset(g_memzone_flags, 2*i) |
              (bitset_isset(g_memzone_flags, 2*i+1)<<1);
}

static void
memzone_set_flags(os_index_t i, int flags)
{
        bitset_setto(g_memzone_flags, 2*i,   flags&(1<<0));
        bitset_setto(g_memzone_flags, 2*i+1, flags&(1<<1));
}

static void
memzone_add_flags(os_index_t i, int flags)
{
        memzone_set_flags(i, memzone_get_flags(i)|flags);
}

static void
memzone_del_flags(os_index_t i, int flags)
{
        memzone_set_flags(i, memzone_get_flags(i)^flags);
}

int
allocator_init(struct page_directory *pd)
{
        const struct virtmem_area *area;
        int err;
        ssize_t pgindex;
        size_t memsz; /* size of memory */

        area = virtmem_area_get_by_name(VIRTMEM_AREA_KERNEL);

        memsz = page_memory(area->npages);

        pgindex = virtmem_alloc_pages_in_area(pd,
                                              1024, /* one largepage */
                                              VIRTMEM_AREA_KERNEL,
                                              PTE_FLAG_PRESENT|
                                              PTE_FLAG_WRITEABLE);
        if (pgindex < 0) {
                err = pgindex;
                goto err_virtmem_alloc_page_in_area;
        }

        g_memzone_flagslen = 1024*PAGE_SIZE;
        g_memzone_flags = page_address(pgindex);
        g_nmemzones = BITS_PER_LARGEPAGE / BITS_PER_PAGE;
        g_memzone_size = memsz / g_nmemzones;

        return 0;

err_virtmem_alloc_page_in_area:
        return err;
}

static os_index_t
memzone_search_unused(size_t nbytes)
{
        int err;
        size_t nmemzones;
        os_index_t mzoff;
        int avail;

        nmemzones = 1 + (nbytes / g_memzone_size);
        mzoff = 0;

        while (mzoff < (g_nmemzones-nmemzones)) {
                size_t j;

                j = 0;

                do {
                        int flags = memzone_get_flags(mzoff+j);

                        avail = (flags&MEMZONE_FLAG_ALLOCED) &
                               !(flags&MEMZONE_FLAG_INUSE);
                        ++j;
                } while (avail && (j < nmemzones));

                if (!avail) {
                        break;
                }

                mzoff += j;
        }

        if (!avail) {
                err = -EAGAIN;
                goto err_avail;
        }

        return mzoff;

err_avail:
        return err;
}

static os_index_t
memzone_find_unused(struct page_directory *pd, size_t nbytes)
{
        ssize_t err;
        os_index_t mzoff;

        if ((mzoff = memzone_search_unused(nbytes)) < 0) {
                const struct virtmem_area *area;
                size_t pgcount;
                ssize_t pgindex;
                size_t nmemzones;
                size_t i;

                if (mzoff != -EAGAIN) {
                        err = mzoff;
                        goto err_memzone_search_unused;
                }

                pgcount = page_count(0, nbytes);

                pgindex = virtmem_alloc_pages_in_area(pd, pgcount,
                                                      VIRTMEM_AREA_KERNEL,
                                                      PTE_FLAG_PRESENT|
                                                      PTE_FLAG_WRITEABLE);
                if (pgindex < 0) {
                        err = pgindex;
                        goto err_memzone_search_unused;
                }

                area = virtmem_area_get_by_name(VIRTMEM_AREA_KERNEL);

                mzoff = page_memory(pgindex-area->pgindex) / g_memzone_size;

                nmemzones = pgcount / g_memzone_size;

                for (i = mzoff; nmemzones; --nmemzones) {
                        memzone_set_flags(i, MEMZONE_FLAG_ALLOCED);
                }
        }

        return mzoff;

err_memzone_search_unused:
        return err;
}

os_index_t
memzone_alloc(struct page_directory *pd, size_t nbytes)
{
        ssize_t err;
        os_index_t mzoff;
        ssize_t nmemzones;
        ssize_t i;

        /* find allocated, but unused memory zones */

        if ((mzoff = memzone_find_unused(pd, nbytes)) < 0) {
                err = mzoff;
                goto err_memzone_find_unused;
        }

        /* mark memory zones as used */

        nmemzones = 1 + (nbytes / g_memzone_size);

        for (i = 0; i < nmemzones; ++i) {
                memzone_add_flags(mzoff+i, MEMZONE_FLAG_INUSE);
        }

        return mzoff;

err_memzone_find_unused:
        return err;
}

static void
memzone_free(struct page_directory *pd, os_index_t mzoff, size_t nbytes)
{
        ssize_t nmemzones;
        ssize_t i;

        /* mark memory zones as unused */

        nmemzones = 1 + (nbytes / g_memzone_size);

        for (i = 0; i < nmemzones; ++i) {
                memzone_del_flags(mzoff+i, MEMZONE_FLAG_INUSE);
        }
}

static void*
memzone_address(os_index_t mzoff)
{
        const struct virtmem_area *area =
                virtmem_area_get_by_name(VIRTMEM_AREA_KERNEL);

        return ((unsigned char*)page_address(area->pgindex)) + 
                        mzoff*g_memzone_size;
}

void *
kmalloc(struct page_directory *pd, size_t nbytes)
{
        ssize_t mzoff;
        size_t *mem;

        if ((mzoff = memzone_alloc(pd, nbytes+2*sizeof(size_t))) < 0) {
                goto err_memzone_alloc;
        }

        mem = memzone_address(mzoff);

        mem[0] = mzoff;
        mem[1] = nbytes;

        return mem+2;

err_memzone_alloc:
        return NULL;
}

void *
kcalloc(struct page_directory *pd, size_t nmemb, size_t nbytes)
{
        void *mem;

        if (!(mem = kmalloc(pd, nmemb*nbytes))) {
                return NULL;
        }

        return memset(mem, 0, nmemb*nbytes);
}

void
kfree(struct page_directory *pd, void *mem)
{
        size_t *mem2 = mem;

        memzone_free(pd, mem2[-2], mem2[-1]);
}

