
#ifndef SYSCALL_H
#define SYSCALL_H

int
syscall_crt_write(const void *buf, unsigned long count, unsigned char attr);

int
syscall_crt_getmaxpos(unsigned short *row, unsigned short *col);

int
syscall_crt_getpos(unsigned short *row, unsigned short *col);

int
syscall_crt_setpos(unsigned short row, unsigned short col);

#endif

