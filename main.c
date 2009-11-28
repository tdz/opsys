
#include "multiboot.h"
#include "console.h"

void
os_main_from_multiboot(struct multiboot_info *mb_info)
{
        console_printf("%s...\n\t%s", "OS kernel booting", "Cool, isn't it?");

	return;
}

