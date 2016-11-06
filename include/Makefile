
topdir = ..
include ../variables.mk
include ../flags.mk

SUBDIRS = opsys

include ../targets.mk
include ../rules.mk

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

