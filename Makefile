
topdir   := .
builddir := $(topdir)/build
srcdir   := $(topdir)/src
outdir   := $(topdir)/out

FEATURES := ctags doxygen image

HOST_CPU := i386

# set flags for the compiler toolchain
CPPFLAGS := -nostdinc
CFLAGS   := -g -m32 -Wall -Werror -ansi -march=$(HOST_CPU) -fno-stack-protector
ASFLAGS := --32 -march=$(HOST_CPU)
LDFLAGS := -nostdlib -static -melf_$(HOST_CPU)

include $(builddir)/vars.mk

# include all modules into build
module := module.mk
include $(shell find -P $(srcdir) -type f -name "$(module)")

CLEAN_ALL_FILES += doc/

include $(builddir)/targets.mk
