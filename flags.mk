
# some default flags needed in all makefiles

SUBDIRS =

ASMSOURCES =
CSOURCES =

LIBRARIES =
PROGRAMS =

EXTRA_CLEAN = tags TAGS

# shell

SHELL = /bin/bash

# compiler

CC = gcc
CPPFLAGS = -nostdinc -I. -I$(srcdir)/include
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
MKDIR = mkdir
RANLIB = ranlib
RM = rm
SED = sed

