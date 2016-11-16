
archdir    := arch/$(HOST_CPU)/
driversdir := drivers/

BINS += oskernel

oskernel_MODULEDIR := kernel/bin

oskernel_SRCS = alloc.c \
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

oskernel_INCLUDES += kernel/bin/$(archdir) kernel/bin
oskernel_INCLUDES += kernel/include/$(archdir) kernel/include
oskernel_LD_SEARCH_PATHS += kernel/bin kernel/lib

# include architecture-specific files
include $(srcdir)/kernel/bin/$(archdir)/arch.mk

# include drivers
driver_mk := driver.mk
include $(shell find -P $(srcdir)/kernel/bin/$(driversdir) -type f -name "$(driver_mk)")

oskernel_LIBS += libopsys.a
