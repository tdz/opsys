
topdir = ../../../..
include ../../../variables.mk
include ../../../flags.mk

#include ../../variables.mk
#include ../../flags.mk

# kernel resides at 1 MiB
reloc_addr = 0x100000

ASMSOURCES = entry.S \
             idt.S \
             multiboot.S \
             tcb.S

CSOURCES = entry.c \
           gdt.c \
           idt.c \
           idtentry.c \
           interupt.c \
           ioports.c \
           pic.c \
           tcb.c

CPPFLAGS += -I$(srcdir)/kernel

LDFLAGS += -Ttext=$(reloc_addr) -L$(srcdir)/kernel -e_start

LDADD += -lkernel

.PHONY : all clean

all : oskernel

clean :
	rm -fr oskernel
	rm -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)

oskernel : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	ld $(LDFLAGS) -o $@ $? $(LDADD)

include ../../../rules.mk

