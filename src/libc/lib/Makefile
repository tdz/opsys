
topdir := ../../..
builddir := $(topdir)/build
include $(builddir)/variables.mk
include $(builddir)/flags.mk

ASMSOURCES = start.S

CSOURCES = syscall.c \
           syscall0.c \
           write.c

CPPFLAGS += -I$(includedir)/libc -I$(includedir)/opsys

LIBRARIES = libc.a

include $(builddir)/targets.mk
include $(builddir)/rules.mk
include $(builddir)/deps.mk
