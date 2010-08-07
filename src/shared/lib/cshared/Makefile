
topdir = ../../../..
include ../../../variables.mk
include ../../../flags.mk

include ../../variables.mk
include ../../flags.mk

ASMSOURCES =

CSOURCES = errno.c \
           string.c

CPPFLAGS += -I$(includedir)

LIBRARIES = libcshared.a

.PHONY : all clean

all : $(LIBRARIES)

clean :
	$(RM) -fr $(LIBRARIES)
	$(RM) -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o) $(CSOURCES:.c=.d)

include ../../../rules.mk

# include dependency rules
-include $(CSOURCES:.c=.d)

