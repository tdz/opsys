
#include "types.h"
#include "syscall.h"
#include "idt.h"

enum {
        IDT_FLAG_SEGINMEM = 0x80,
        IDT_FLAG_32BITINT = 0x0e,
        IDT_FLAG_16BITINT = 0x06
};

struct idt_entry
{
        unsigned short base_low;
        unsigned short tss;
        unsigned char  reserved;
        unsigned char  flags;
        unsigned short base_high;
};

static void
idt_entry_init(struct idt_entry *idte, unsigned long  func,
                                       unsigned short tss,
                                       unsigned char  ring,
                                       unsigned char  flags)
{
        idte->base_low = func&0xffff;
        idte->tss = tss;
        idte->reserved = 0;
        idte->flags = flags | ((ring&0x03)<<7);
        idte->base_high = (func>>16)&0xffff;
}

#include "console.h"

static void
default_handler(void)
{
        console_printf("default handler\n");
        return;
}

static struct idt_entry g_idt[2];

void
idt_init(void)
{
        size_t i;

        for (i = 0; i < sizeof(g_idt)/sizeof(g_idt[0]); ++i) {
                idt_entry_init(g_idt+i, (unsigned long)default_handler,
                                        0x8,
                                        0,
                                        IDT_FLAG_SEGINMEM|
                                        IDT_FLAG_32BITINT);
        }
}

struct idt_register
{
        unsigned short limit __attribute__ ((packed));
        unsigned long  base  __attribute__ ((packed));
};

const static struct idt_register idtr __asm__ ("_idtr") = {
        .limit = sizeof(g_idt),
        .base  = (unsigned long)g_idt,
};

void
idt_install()
{
        __asm__ ("lidt (_idtr)\n\t");
}

