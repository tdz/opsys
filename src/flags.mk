
# some default flags needed in all makefiles

SHELL = /bin/bash

# compiler

CC ?= gcc
CPPFLAGS = -nostdinc -I. -I$(srcdir)/shared/include
CFLAGS = -g -m32 -Wall -ansi -march=$(target_cpu) -fno-stack-protector

# assembler

AS ?= as
ASFLAGS = --32 -march=$(target_cpu)

# linker

LD ?= ld
LDFLAGS = -nostdlib -static -melf_$(target_cpu)

# others

AR ?= ar
RANLIB ?= ranlib
RM ?= rm
