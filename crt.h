
#ifndef CRT_H
#define CRT_H

ssize_t
crt_write(volatile unsigned char *vidmem,
          const void *buf,
          size_t count,
          unsigned char attr);

int
crt_getpos(unsigned short *row, unsigned short *col);

int
crt_setpos(unsigned short row, unsigned short col);

volatile unsigned char *
crt_getaddress(unsigned short row, unsigned short col);

#endif

