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

#include <stddef.h>
#include <string.h>
#include <sys/types.h>

/* virtual memory */
#include <vmemarea.h>

#include "memzone.h"
#include "alloc.h"

static struct memzone g_memzone_kernel;

int
allocator_init(struct address_space *as)
{
        return memzone_init(&g_memzone_kernel, as, VIRTMEM_AREA_KERNEL);
}

void *
kmalloc(size_t nbytes)
{
        ssize_t chunk;
        size_t *mem;
        size_t nchunks;

        nchunks = memzone_get_nchunks(&g_memzone_kernel,
                                      nbytes + 2 * sizeof(size_t));

        if ((chunk = memzone_alloc(&g_memzone_kernel, nchunks)) < 0)
        {
                goto err_memzone_alloc;
        }

        mem = memzone_address(&g_memzone_kernel, chunk);

        mem[0] = chunk;
        mem[1] = nbytes;

        return mem + 2;

err_memzone_alloc:
        return NULL;
}

void *
kcalloc(size_t nmemb, size_t nbytes)
{
        void *mem;

        if (!(mem = kmalloc(nmemb * nbytes)))
        {
                return NULL;
        }

        return memset(mem, 0, nmemb * nbytes);
}

void
kfree(void *mem)
{
        size_t *mem2 = mem;

        memzone_free(&g_memzone_kernel, mem2[-2],
                     memzone_get_nchunks(&g_memzone_kernel, mem2[-1]));
}
