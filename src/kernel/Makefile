
topdir = ../..
include ../variables.mk
include ../flags.mk

archdir = ./arch

SUBDIRS = $(archdir)

ASMSOURCES =

CSOURCES = alloc.c \
           bitset.c \
           console.c \
           crt.c \
           elfldr.c \
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
           semaphore.c \
           spinlock.c \
           syscall.c \
           syssrv.c \
           task.c \
           taskhlp.c \
           tcbhlp.c \
           virtmem.c \
           vmemarea.c

LDFLAGS += -Ttext=$(reloc_addr) -L$(archdir)/$(target_cpu)

CPPFLAGS += -I$(archdir)/$(target_cpu) -I./include -I$(srcdir)/shared/include/opsys

LIBRARIES = libkernel.a

.PHONY : all clean

all : $(LIBRARIES)
	for dir in $(SUBDIRS); do ($(MAKE) -C $$dir all); done

clean :
	for dir in $(SUBDIRS); do ($(MAKE) -C $$dir clean); done
	$(RM) -fr $(LIBRARIES)
	$(RM) -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o) $(CSOURCES:.c=.d)

include ../rules.mk

# include dependency rules
-include $(CSOURCES:.c=.d)

