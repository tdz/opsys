
BUILDDIR := build
SRCDIR   := src
OUTDIR   := out

FEATURES := ctags doxygen image

HOST_CPU := i386

# set flags for the compiler toolchain
CPPFLAGS := -nostdinc
CFLAGS   := -g -std=c11 -Wall -Werror -m32 -march=$(HOST_CPU) -fno-stack-protector
ASFLAGS := --32 -march=$(HOST_CPU)
LDFLAGS := -nostdlib -static -melf_$(HOST_CPU)

include $(BUILDDIR)/vars.mk

# include all modules into build
module := module.mk
include $(shell find -P $(srcdir) -type f -name "$(module)")

CLEAN_ALL_FILES += doc/

include $(BUILDDIR)/targets.mk
