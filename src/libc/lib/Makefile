
topdir := ../../..
builddir := $(topdir)/build
include $(builddir)/variables.mk
include $(builddir)/flags.mk

LIBS = libc.a

libc.a_ASMSOURCES = start.S

libc.a_CSOURCES = syscall.c \
                  syscall0.c \
                  write.c

libc.a_CPPFLAGS += -I$(includedir)/libc -I$(includedir)/opsys

include $(builddir)/targets.mk
include $(builddir)/deps.mk
