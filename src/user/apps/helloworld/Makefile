
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

.PHONY : all clean

all : helloworld

clean :
	rm -fr helloworld
	rm -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o) $(CSOURCES:.c=.d)

helloworld : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	ld $(LDFLAGS) -o $@ $^ -lcrt $(LDADD)

include ../../../rules.mk

# include dependency rules
-include $(CSOURCES:.c=.d)

