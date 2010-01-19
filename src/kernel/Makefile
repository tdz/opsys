
topdir = ../..
include ../variables.mk
include ../flags.mk

archdir = ./arch

ASMSOURCES =

CSOURCES = console.c \
           crt.c \
           elfldr.c \
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
           virtmem.c

LDFLAGS += -Ttext=$(reloc_addr) -L$(archdir)/$(target_cpu)

CPPFLAGS += -I$(archdir)/$(target_cpu)

.PHONY : all clean

all : libkernel.a
	make -C $(archdir) all

clean :
	make -C $(archdir) clean
	rm -fr libkernel.a
	rm -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)

libkernel.a : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	ar rcs $@ $?
	ranlib $@

include ../rules.mk

