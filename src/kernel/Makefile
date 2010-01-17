
topdir = ../..
include ../variables.mk
include ../flags.mk

# kernel resides at 1 MiB
reloc_addr = 0x100000

ASMSOURCES = idt.S \
             multiboot.S

CSOURCES = console.c \
           crt.c \
           elfldr.c \
           gdt.c \
           idt.c \
           interupt.c \
           ioports.c \
           multiboot.c \
           physmem.c \
           pagedir.c \
           pagetbl.c \
           pde.c \
           pit.c \
           pte.c \
           string.c \
           syscall.c \
           task.c \
           tcb.c \
           virtmem.c

LDFLAGS += -Ttext=$(reloc_addr)

.PHONY : all clean

all : oskernel

clean :
	rm -fr oskernel
	rm -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)

oskernel : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	ld $(LDFLAGS) -o $@ $?

include ../rules.mk

