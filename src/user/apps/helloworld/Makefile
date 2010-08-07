
topdir = ../../../..
-include ../../../variables.mk
-include ../../../flags.mk
-include ../../variables.mk
-include ../../flags.mk
-include ../variables.mk
-include ../flags.mk

CSOURCES = main.c

LDFLAGS += -L$(libdir)/crt

PROGRAMS = helloworld

helloworld : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	$(LD) $(LDFLAGS) -o $@ $^ -lcrt $(LDADD)

include ../../../rules.mk

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

