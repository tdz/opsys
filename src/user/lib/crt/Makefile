
topdir = ../../../..
include ../../../variables.mk
include ../../../flags.mk

include ../../variables.mk
include ../../flags.mk

ASMSOURCES =

CSOURCES = write.c

CPPFLAGS += -I$(includedir)/crt

.PHONY : all clean

all : libcrt.a

clean :
	rm -fr libcrt.a
	rm -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o) $(CSOURCES:.c=.d)

libcrt.a : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	ar rcs $@ $^

include ../../../rules.mk

# include dependency rules
-include $(CSOURCES:.c=.d)

