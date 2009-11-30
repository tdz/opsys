
#include "multiboot.h"
#include "console.h"

void
os_main_from_multiboot(struct multiboot_info *mb_info)
{
        console_printf("%s...\n\t%s\n", "OS kernel booting", "Cool, isn't it?");

        /* Setup the global descriptor table for protected mode */

        gdt_init();
        gdt_install();

        /* Setup the interrupt table for protected mode */

        idt_init();
        idt_install();

	return;
}

