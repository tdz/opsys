
topdir = ../../..
include ../../../variables.mk
include ../../../flags.mk
-include ../../variables.mk
-include ../../flags.mk
-include ../variables.mk
-include ../flags.mk

ASMSOURCES = start.S

CSOURCES = syscall.c \
           syscall0.c \
           write.c

CPPFLAGS += -I$(includedir)/libc -I$(includedir)/opsys

LIBRARIES = libc.a

include ../../../rules.mk

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

