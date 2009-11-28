
#include "multiboot.h"

#include "types.h"
#include "crt.h"

#include "string.h"

#define HELLO_WORLD     "Hello world!"

void
os_main_from_multiboot(struct multiboot_info *mb_info)
{
        unsigned short row, col;

        crt_getpos(&row, &col);

        crt_write(crt_getaddress(row, col), HELLO_WORLD, strlen(HELLO_WORLD), 0x7);

        crt_setpos(1, 1);

	return;
}

