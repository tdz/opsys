
#include "multiboot.h"
#include "console.h"
#include "gdt.h"
#include "idt.h"

void
os_main_from_multiboot(struct multiboot_info *mb_info)
{
        console_printf("%s...\n\t%s\n", "OS kernel booting",
                                        "Cool, isn't it?");

        /* setup GDT for protected mode */
        gdt_init();
        gdt_install();

        /* setup IDT for protected mode */
        idt_init();
        idt_install();
}

