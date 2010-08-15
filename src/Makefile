
topdir = ..
include ../variables.mk
include ../flags.mk

SUBDIRS = shared kernel user 

include ../rules.mk

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

