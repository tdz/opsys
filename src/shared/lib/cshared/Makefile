
topdir = ../../../..
include ../../../variables.mk
include ../../../flags.mk

include ../../variables.mk
include ../../flags.mk

ASMSOURCES =

CSOURCES = errno.c \
           string.c

CPPFLAGS += -I$(includedir)

.PHONY : all clean

all : libcshared.a

clean :
	rm -fr libcshared.a
	rm -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)

libcshared.a : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	ar rcs $@ $?

include ../../../rules.mk

