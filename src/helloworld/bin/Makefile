
topdir := ../../..
builddir := $(topdir)/build
include $(builddir)/variables.mk
include $(builddir)/flags.mk

BINS += helloworld

helloworld_CSOURCES = main.c

helloworld_CPPFLAGS += -I$(includedir)/libc -I$(includedir)/opsys

helloworld_LDFLAGS += -L$(srcdir)/libc/lib -L$(srcdir)/kernel/lib

helloworld_LDADD := -lc -lopsys

include $(builddir)/deps.mk
include $(builddir)/targets.mk
