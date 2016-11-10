
# some default flags needed in all makefiles

BINS =
FILES =
LIBS =

EXTRA_CLEAN = tags TAGS

DEPSDIR = .deps

# shell

SHELL = /bin/bash

# compiler

CC = gcc
CPPFLAGS = -nostdinc
CFLAGS = -g -m32 -Wall -Werror -ansi -march=$(target_cpu) -fno-stack-protector

# assembler

AS = as
ASFLAGS = --32 -march=$(target_cpu)

# linker

LD = ld
LDFLAGS = -nostdlib -static -melf_$(target_cpu)

# ctags

CTAGS = ctags
CTAGSFLAGS =

# others

AR = ar
CP = cp
LN = ln
MKDIR = mkdir
RANLIB = ranlib
RM = rm
SED = sed
TOUCH := touch
