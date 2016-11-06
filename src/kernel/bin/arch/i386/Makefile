
topdir = ../../../../..
include ../../../../../variables.mk
include ../../../../../flags.mk
-include ../../../../variables.mk
-include ../../../../flags.mk
-include ../../../variables.mk
-include ../../../flags.mk
-include ../../variables.mk
-include ../../flags.mk
-include ../variables.mk
-include ../flags.mk

# kernel resides at 1 MiB
reloc_addr = 0x100000

ASMSOURCES = idt.S \
             multiboot.S \
             tcbregs.S

CSOURCES = debug.c \
           gdt.c \
           idt.c \
           idtentry.c \
           interupt.c \
           ioports.c \
           main.c \
           multiboot.c \
           pagedir.c \
           pagetbl.c \
           pde.c \
           pic.c \
           pte.c \
           tcb.c \
           tcbregs.c \
           vmem.c \
           vmem_32.c \
           vmem_pae.c \
           vmemhlp.c

CPPFLAGS += -I$(srcdir)/kernel/include -I$(srcdir)/kernel/bin -I$(includedir)/opsys

LDFLAGS += -Ttext=$(reloc_addr) -L$(srcdir)/kernel/bin -L$(srcdir)/kernel/lib -e_start

LDADD := -lkernel -lopsys $(LDADD)

PROGRAMS = oskernel

include ../../../../../targets.mk
include ../../../../../rules.mk

oskernel : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDADD)

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

