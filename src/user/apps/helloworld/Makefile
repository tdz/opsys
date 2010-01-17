
topdir = ../../../..
include ../../../variables.mk
include ../../../flags.mk

include ../../variables.mk
include ../../flags.mk

include ../variables.mk
include ../flags.mk

CSOURCES = main.c

.PHONY : all clean

all : helloworld

clean :
	rm -fr helloworld
	rm -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)

helloworld : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	ld $(LDFLAGS) -o $@ $? $(LDADD)

include ../../../rules.mk

