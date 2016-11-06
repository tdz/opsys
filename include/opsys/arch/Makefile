
topdir := ../../..
builddir := $(topdir)/build
include $(builddir)/variables.mk
include $(builddir)/flags.mk

.PHONY = FORCE

FILES = target

target : FORCE
	$(LN) -f -s $(target_cpu) $@

FORCE :

include $(builddir)/deps.mk
include $(builddir)/rules.mk
include $(builddir)/targets.mk
