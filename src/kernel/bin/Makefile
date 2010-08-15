
topdir = ../../..
include ../../../variables.mk
include ../../../flags.mk
-include ../../variables.mk
-include ../../flags.mk
-include ../variables.mk
-include ../flags.mk

archdir = arch

SUBDIRS = $(archdir)

CSOURCES = alloc.c \
           assert.c \
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
           pit.c \
           pmem.c \
           pmemarea.c \
           sched.c \
           semaphore.c \
           spinlock.c \
           syscall.c \
           syssrv.c \
           task.c \
           taskhlp.c \
           tcbhlp.c \
           vmemarea.c

LDFLAGS += -Ttext=$(reloc_addr) -L$(archdir)/$(target_cpu)

CPPFLAGS += -I$(archdir)/$(target_cpu) -I$(includedir) -I$(includedir)/opsys/$(archdir)/$(target_cpu) -I$(includedir)/opsys

LIBRARIES = libkernel.a

include ../../../rules.mk

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

