
topdir := ../../..
builddir := $(topdir)/build
include $(builddir)/variables.mk
include $(builddir)/flags.mk

BINS += helloworld

helloworld_SRCS = main.c

helloworld_INCLUDES += $(includedir)/libc $(includedir)/opsys

helloworld_LD_SEARCH_PATHS += $(srcdir)/libc/lib $(srcdir)/kernel/lib

helloworld_LDADD := -lc -lopsys

include $(builddir)/deps.mk
include $(builddir)/targets.mk
