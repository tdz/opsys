
topdir = ../..
include ../../variables.mk
include ../../flags.mk
-include ../variables.mk
-include ../flags.mk

SUBDIRS = arch

include ../../rules.mk

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

