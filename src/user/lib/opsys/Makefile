
topdir = ../../../..
include ../../../../variables.mk
include ../../../../flags.mk
-include ../../../variables.mk
-include ../../../flags.mk
-include ../../variables.mk
-include ../../flags.mk
-include ../variables.mk
-include ../flags.mk

ASMSOURCES = start.S

CSOURCES = syscall.c \
           syscall0.c

CPPFLAGS += -I$(includedir)/opsys

LIBRARIES = libopsys.a

include ../../../../rules.mk

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

