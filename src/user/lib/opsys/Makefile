
topdir = ../../../..
include ../../../variables.mk
include ../../../flags.mk

include ../../variables.mk
include ../../flags.mk

ASMSOURCES = start.S

CSOURCES = syscall.c \
           syscall0.c

CPPFLAGS += -I$(includedir)/opsys

.PHONY : all clean

all : libopsys.a

clean :
	rm -fr libopsys.a
	rm -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o) $(CSOURCES:.c=.d)

libopsys.a : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	ar rcs $@ $^

include ../../../rules.mk

# include dependency rules
-include $(CSOURCES:.c=.d)

