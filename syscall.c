
#include "types.h"
#include "syscall.h"
#include "crt.h"

int
syscall_crt_write(const void *buf, unsigned long count, unsigned char attr)
{
        unsigned short row, col;
        volatile unsigned char *vidmem;

        crt_getpos(&row, &col);
        vidmem = crt_getaddress(row, col);

        return crt_write(vidmem, buf, (size_t)count, attr);
}

int
syscall_crt_getmaxpos(unsigned short *row, unsigned short *col)
{
        return 0;
}

int
syscall_crt_getpos(unsigned short *row, unsigned short *col)
{
        return 0;
}

int
syscall_crt_setpos(unsigned short row, unsigned short col)
{
        return 0;
}

