
topdir = ../../../..
include ../../../variables.mk
include ../../../flags.mk

#include ../../variables.mk
#include ../../flags.mk

# kernel resides at 1 MiB
reloc_addr = 0x100000

ASMSOURCES = idt.S \
             multiboot.S \
             tcb.S

CSOURCES = addrspace.c \
           debug.c \
           gdt.c \
           idt.c \
           idtentry.c \
           interupt.c \
           ioports.c \
           pagedir.c \
           pagetbl.c \
           pde.c \
           pic.c \
           pte.c \
           tcb.c

CPPFLAGS += -I$(srcdir)/kernel/include -I$(srcdir)/kernel

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

