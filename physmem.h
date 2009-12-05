
#ifndef PHYSMEM_H
#define PHYSMEM_H

#define PAGE_SHIFT      12
#define PAGE_SIZE       (1<<PAGE_SHIFT)
#define PAGE_MASK       (PAGE_SIZE-1)

int
physmem_init(unsigned long npages);

int
physmem_add_area(unsigned long pgoffset,
                 unsigned long npages,
                 unsigned char flags);

#endif

