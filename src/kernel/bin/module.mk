
archdir    := arch/$(HOST_CPU)/
driversdir := drivers/

BINS += kernel

kernel_MODULEDIR := kernel/bin

kernel_SRCS = alloc.c \
              assert.c \
              bitset.c \
              console.c \
              elfldr.c \
              ipc.c \
              ipcmsg.c \
              irq.c \
              list.c \
              loader.c \
              memzone.c \
              pmem.c \
              pmemarea.c \
              sched.c \
              semaphore.c \
              spinlock.c \
              syscall.c \
              sysexec.c \
              syssrv.c \
              task.c \
              taskhlp.c \
              tcb.c \
              tcbhlp.c \
              timer.c \
              vmem.c \
              vmemarea.c

kernel_INCLUDES += kernel/bin/$(archdir) \
                   kernel/bin \
                   kernel/include/$(archdir) \
                   kernel/include \
                   libc0/include
kernel_LD_SEARCH_PATHS += libc0/lib

# include architecture-specific files
include $(srcdir)/kernel/bin/$(archdir)/arch.mk

# include drivers
driver_mk := driver.mk
include $(shell find -P $(srcdir)/kernel/bin/$(driversdir) -type f -name "$(driver_mk)")

kernel_LIBS += libc0.a
