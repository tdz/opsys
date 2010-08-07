
topdir = ../../../..
include ../../../variables.mk
include ../../../flags.mk

#include ../../variables.mk
#include ../../flags.mk

# kernel resides at 1 MiB
reloc_addr = 0x100000

ASMSOURCES = idt.S \
             multiboot.S \
             tcbregs.S

CSOURCES = addrspace.c \
           addrspacehlp.c \
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
           tcb.c \
           tcbregs.c

CPPFLAGS += -I$(srcdir)/kernel/include -I$(srcdir)/kernel

LDFLAGS += -Ttext=$(reloc_addr) -L$(srcdir)/kernel -L$(srcdir)/shared/lib/opsysshared -L$(srcdir)/shared/lib/cshared -e_start

LDADD += -lkernel -lopsysshared -lcshared

PROGRAMS = oskernel

.PHONY : all clean

all : $(PROGRAMS)

clean :
	$(RM) -fr $(PROGRAMS)
	$(RM) -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o) $(CSOURCES:.c=.d)

oskernel : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDADD)

include ../../../rules.mk

# include dependency rules
-include $(CSOURCES:.c=.d)

