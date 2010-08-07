
topdir = ../../../..
-include ../../../variables.mk
-include ../../../flags.mk
-include ../../variables.mk
-include ../../flags.mk
-include ../variables.mk
-include ../flags.mk

CSOURCES = tid.c

CPPFLAGS += -I$(includedir) -I$(srcdir)/shared/include/opsys

LIBRARIES = libopsysshared.a

include ../../../rules.mk

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

