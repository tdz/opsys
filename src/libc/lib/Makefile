
topdir := ../../..
builddir := $(topdir)/build
include $(builddir)/variables.mk
include $(builddir)/flags.mk

LIBS = libc.a

libc.a_SRCS = start.S \
              syscall.c \
              syscall0.c \
              write.c

libc.a_INCLUDES += $(includedir)/libc $(includedir)/opsys

include $(builddir)/targets.mk
include $(builddir)/deps.mk
