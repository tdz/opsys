
topdir = ../../../..
include ../../../../variables.mk
include ../../../../flags.mk
-include ../../../variables.mk
-include ../../../flags.mk
-include ../../variables.mk
-include ../../flags.mk
-include ../variables.mk
-include ../flags.mk

CSOURCES = main.c

LDFLAGS += -L$(libdir)/crt

LDADD := -lcrt $(LDADD)

PROGRAMS = helloworld

include ../../../../rules.mk

helloworld : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDADD)

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

