
#include "gdt.h"

struct gdt_entry
{
        unsigned short limit_low;
        unsigned short base_low;
        unsigned char  base_mid;
        unsigned char  flags;
        unsigned char  limit_high;
        unsigned char  base_high;
};

static void
gdt_entry_init(struct gdt_entry *gdte, unsigned long base,
                                       unsigned long limit,
                                       unsigned char flags)
{
        gdte->limit_low  = limit&0xffff;
        gdte->base_low   = base&0xffff;
        gdte->base_mid   = (base>>16)&0xff;
        gdte->flags      = flags;
        gdte->limit_high = (limit>>16)&0x0f | 0xc0;
        gdte->base_high  = (base>>24)&0xff;
}

static struct gdt_entry g_gdt[3];

void
gdt_init()
{
        gdt_entry_init(g_gdt+0, 0, 0, 0);
        gdt_entry_init(g_gdt+1, 0x00000000,
                                0x00ffffff,
                                0x9a/*10011010*/);
        gdt_entry_init(g_gdt+2, 0x00000000,
                                0x00ffffff,
                                0x92/*10010010*/);
}

struct gdt_register
{
        unsigned short limit __attribute__ ((packed));
        unsigned long  base  __attribute__ ((packed)); 
};

static const struct gdt_register gdtr __asm__ ("_gdtr") = {
        .limit = sizeof(g_gdt),
        .base  = (unsigned long )g_gdt,
};

void
gdt_install()
{
        __asm__ ("lgdt (_gdtr)\n\t");
}

