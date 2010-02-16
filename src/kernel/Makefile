
topdir = ../..
include ../variables.mk
include ../flags.mk

archdir = ./arch

ASMSOURCES =

CSOURCES = alloc.c \
           bitset.c \
           console.c \
           crt.c \
           elfldr.c \
           errno.c \
           ipc.c \
           ipcmsg.c \
           kbd.c \
           list.c \
           loader.c \
           memzone.c \
           multiboot.c \
           physmem.c \
           pit.c \
           sched.c \
           string.c \
           syscall.c \
           syssrv.c \
           task.c \
           taskhlp.c \
           tcbhlp.c \
           tid.c \
           virtmem.c \
           vmemarea.c

LDFLAGS += -Ttext=$(reloc_addr) -L$(archdir)/$(target_cpu)

CPPFLAGS += -I$(archdir)/$(target_cpu) -I./include 

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

