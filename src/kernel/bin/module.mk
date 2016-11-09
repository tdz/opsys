
archdir = arch/$(target_cpu)

BINS += oskernel

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

oskernel_INCLUDES += $(archdir)
oskernel_INCLUDES += $(includedir)/opsys/$(archdir) $(includedir)/opsys $(includedir)
oskernel_INCLUDES += $(srcdir)/kernel/include $(srcdir)/kernel/bin
oskernel_LD_SEARCH_PATHS += $(srcdir)/kernel/bin $(srcdir)/kernel/lib

# include architecture-specific files
include $(archdir)/arch.mk

oskernel_LDADD := -lopsys
