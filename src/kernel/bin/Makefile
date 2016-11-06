
topdir = ../../..
include ../../../variables.mk
include ../../../flags.mk
-include ../../variables.mk
-include ../../flags.mk
-include ../variables.mk
-include ../flags.mk

archdir = arch/$(target_cpu)

ASMSOURCES =

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

CPPFLAGS += -I$(archdir)
CPPFLAGS += -I$(includedir)/opsys/$(archdir) -I$(includedir)/opsys -I$(includedir)
CPPFLAGS += -I$(srcdir)/kernel/include -I$(srcdir)/kernel/bin
LDFLAGS += -L$(srcdir)/kernel/bin -L$(srcdir)/kernel/lib

# include architecture-specific files
include $(archdir)/arch.mk

LDADD := -lopsys $(LDADD)

PROGRAMS = oskernel

oskernel : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDADD)

include ../../../targets.mk
include ../../../rules.mk

# include dependency rules
-include $(CSOURCES:%.c=$(DEPSDIR)/%.d)
