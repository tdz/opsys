
#include "ioports.h"

void
io_outb(unsigned short port, unsigned char byte)
{
        __asm__("mov %1, %%al\n         \
                 mov %0, %%dx\n         \
                 out %%al, %%dx\n"
                        :
                        : "r" (port), "r" (byte)
                        : "%al", "%dx"
                );
}

void
io_outb_index(unsigned short iport, unsigned char index,
              unsigned short dport, unsigned char byte)
{
        io_outb(iport, index);
        io_outb(dport, byte);
}

void
io_inb(unsigned short port, unsigned char *byte)
{
        __asm__("mov %1, %%dx\n         \
                 in %%dx, %%al\n        \
                 mov %%al, %0\n"
                        : "=r" (*byte)
                        : "r" (port)
                        : "%al", "%dx"
                );
}

void
io_inb_index(unsigned short iport, unsigned char index,
             unsigned short dport, unsigned char *byte)
{
        io_outb(iport, index);
        io_inb(dport, byte);
}

