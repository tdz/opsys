
topdir = ../../..
include ../../../variables.mk
include ../../../flags.mk
-include ../../variables.mk
-include ../../flags.mk
-include ../variables.mk
-include ../flags.mk

CSOURCES = errno.c \
           string.c \
           tid.c

CPPFLAGS += -I$(includedir) -I$(includedir)/opsys

LIBRARIES = libopsys.a

include ../../../rules.mk

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

