
# some default flags needed in all makefiles

SHELL = /bin/bash

CC = gcc

CPPFLAGS = -nostdinc -I. -I$(srcdir)/shared/include

CFLAGS = -g -m32 -Wall -ansi -march=$(target_cpu) -fno-stack-protector

ASFLAGS = --32 -march=$(target_cpu)

LDFLAGS = -nostdlib -static -melf_$(target_cpu)

