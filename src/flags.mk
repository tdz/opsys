
# some default flags needed in all makefiles

SHELL = /bin/sh

CPPFLAGS = -nostdinc -I.

CFLAGS = -m32 -Wall -ansi -march=$(target_cpu)

ASFLAGS = --32 -march=$(target_cpu)

LDFLAGS = -nostdlib -static -melf_$(target_cpu)

