
topdir = ../../../..
include ../../../variables.mk
include ../../../flags.mk

include ../../variables.mk
include ../../flags.mk

include ../variables.mk
include ../flags.mk

CSOURCES = main.c

CPPFLAGS +=

LDFLAGS += -L$(libdir)/crt

PROGRAMS = helloworld

.PHONY : all clean

all : $(PROGRAMS)

clean :
	$(RM) -fr $(PROGRAMS)
	$(RM) -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o) $(CSOURCES:.c=.d)

helloworld : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	$(LD) $(LDFLAGS) -o $@ $^ -lcrt $(LDADD)

include ../../../rules.mk

# include dependency rules
-include $(CSOURCES:.c=.d)

