
topdir   := .
builddir := $(topdir)/build
srcdir   := $(topdir)/src
outdir   := $(topdir)/out

FEATURES := ctags doxygen

target_cpu ?= i386

# set flags for the compiler toolchain
CPPFLAGS := -nostdinc
CFLAGS   := -g -m32 -Wall -Werror -ansi -march=$(target_cpu) -fno-stack-protector
ASFLAGS  := --32 -march=$(target_cpu)
LDFLAGS  := -nostdlib -static -melf_$(target_cpu)

include $(builddir)/vars.mk

.DEFAULT_GOAL := all

# include all modules into build
module := module.mk
include $(shell find -P $(srcdir) -type f -name "$(module)")

IMAGES = opsyshdd.img

.PHONY: image

image: $(IMAGES)

%.img : all
	$(CP) res/genimage/$@.base $@
	sudo tools/genimage/genimage.sh -a $(target_cpu) -b res/genimage/ -s $(outdir) -i $@

CLEAN_ALL_FILES += doc/ $(IMAGES)

include $(builddir)/targets.mk
