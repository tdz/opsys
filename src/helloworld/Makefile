
topdir := ../..
builddir := $(topdir)/build
include $(builddir)/variables.mk
include $(builddir)/flags.mk

SUBDIRS = bin

include $(builddir)/targets.mk
include $(builddir)/rules.mk

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

