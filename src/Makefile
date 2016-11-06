
topdir = ..
include ../variables.mk
include ../flags.mk

SUBDIRS = kernel libc helloworld minesweeper

include ../targets.mk
include ../rules.mk

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

