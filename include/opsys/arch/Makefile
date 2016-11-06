
topdir := ../../..
builddir := $(topdir)/build
include $(builddir)/variables.mk
include $(builddir)/flags.mk

.PHONY = FORCE

FILES = target

include $(builddir)/targets.mk
include $(builddir)/rules.mk

target : FORCE
	$(LN) -f -s $(target_cpu) $@

FORCE :
