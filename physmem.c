
#include "types.h"
#include "string.h"
#include "physmem.h"

/* Memory-map offset at 24 MiB */
static unsigned char *physmap = (unsigned char*)0x01800000;
static unsigned long  physmap_npages = 0;

int
physmem_init(unsigned long npages)
{
        memset(physmap, 0, npages*sizeof(physmap[0]));
        physmap_npages = npages;

        return 0;
}

int
physmem_add_area(unsigned long pgoffset,
                 unsigned long npages,
                 unsigned char flags)
{
        unsigned char *physmap = physmap+pgoffset;

        while (npages--) {
                *(physmap++) = (flags&0x3)<<6;
        }

        return 0;
}

