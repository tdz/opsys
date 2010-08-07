
topdir = ../../../..
include ../../../variables.mk
include ../../../flags.mk

include ../../variables.mk
include ../../flags.mk

ASMSOURCES =

CSOURCES = tid.c

CPPFLAGS += -I$(includedir) -I$(srcdir)/shared/include/opsys

.PHONY : all clean

all : libopsysshared.a

clean :
	rm -fr libopsysshared.a
	rm -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o) $(CSOURCES:.c=.d)

libopsysshared.a : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	ar rcs $@ $^

include ../../../rules.mk

# include dependency rules
-include $(CSOURCES:.c=.d)

