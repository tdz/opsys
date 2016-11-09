
topdir := ../../..
builddir := $(topdir)/build
include $(builddir)/variables.mk
include $(builddir)/flags.mk

LIBS += libopsys.a

libopsys.a_CSOURCES = errno.c \
                      string.c \
                      tid.c

libopsys.a_CPPFLAGS += -I$(includedir) -I$(includedir)/opsys

include $(builddir)/targets.mk
include $(builddir)/deps.mk
