
#ifndef IOPORTS_H
#define IOPORTS_H

void
io_outb(unsigned short port, unsigned char byte);

void
io_outb_index(unsigned short iport, unsigned char index,
              unsigned short dport, unsigned char byte);

void
io_inb(unsigned short port, unsigned char *byte);

void
io_inb_index(unsigned short iport, unsigned char index,
             unsigned short dport, unsigned char *byte);

#endif

