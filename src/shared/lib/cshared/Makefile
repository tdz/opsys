
topdir = ../../../..
-include ../../../variables.mk
-include ../../../flags.mk
-include ../../variables.mk
-include ../../flags.mk
-include ../variables.mk
-include ../flags.mk

CSOURCES = errno.c \
           string.c

CPPFLAGS += -I$(includedir)

LIBRARIES = libcshared.a

include ../../../rules.mk

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

