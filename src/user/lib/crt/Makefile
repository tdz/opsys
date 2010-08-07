
topdir = ../../../..
include ../../../../variables.mk
include ../../../../flags.mk
-include ../../../variables.mk
-include ../../../flags.mk
-include ../../variables.mk
-include ../../flags.mk
-include ../variables.mk
-include ../flags.mk

CSOURCES = write.c

CPPFLAGS += -I$(includedir)/crt

LIBRARIES = libcrt.a

include ../../../../rules.mk

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

