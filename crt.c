
#include "stddef.h"
#include "types.h"
#include "ioports.h"

#define MAX_ROW 25
#define MAX_COL 80

static unsigned long
crt_getindex(unsigned short row, unsigned short col)
{
        return col+row*MAX_COL;
}

ssize_t
crt_write(volatile unsigned char *vidmem,
          const void *buf,
          size_t count,
          unsigned char attr)
{
        size_t i;
        const unsigned char *buf8;

        for (buf8 = buf, i = 0; i < count; ++i) {
                *vidmem = *buf8;
                ++buf8;
                ++vidmem;
                *vidmem = attr;
                ++vidmem;
        }

        return i;
}

int
crt_getpos(unsigned short *row, unsigned short *col)
{
        unsigned char curh, curl;
        unsigned long cur;

        io_inb_index(0x3d4, 0x0e, 0x03d5, &curh);
        io_inb_index(0x3d4, 0x0f, 0x03d5, &curl);

        cur = (curh<<8) | curl;

        *row = cur/80;
        *col = cur - (*row)*80;

        return 0;
}

int
crt_getmaxpos(unsigned short *row, unsigned short *col)
{
        *row = MAX_ROW;
        *col = MAX_COL;

        return 0;
}

int
crt_setpos(unsigned short row, unsigned short col)
{
        unsigned long cur = crt_getindex(row, col);

        io_outb_index(0x03d4, 0x0e, 0x03d5, (cur>>8)&0xff);
        io_outb_index(0x03d4, 0x0f, 0x03d5, cur&0xff);

        return 0;
}

volatile unsigned char *
crt_getaddress(unsigned short row, unsigned short col)
{
        if ((row >= MAX_ROW) || (col >= MAX_COL)) {
                return NULL;
        }

        return ((volatile unsigned char*)0xb8000)+2*crt_getindex(row, col);
}

