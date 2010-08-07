
topdir = ../../../..
include ../../../variables.mk
include ../../../flags.mk

include ../../variables.mk
include ../../flags.mk

ASMSOURCES = start.S

CSOURCES = syscall.c \
           syscall0.c

CPPFLAGS += -I$(includedir)/opsys

LIBRARIES = libopsys.a

.PHONY : all clean

all : $(LIBRARIES)

clean :
	$(RM) -fr $(LIBRARIES)
	$(RM) -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o) $(CSOURCES:.c=.d)

include ../../../rules.mk

# include dependency rules
-include $(CSOURCES:.c=.d)

