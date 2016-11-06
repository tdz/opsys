
topdir := ../../..
builddir := $(topdir)/build
include $(builddir)/variables.mk
include $(builddir)/flags.mk

CSOURCES = errno.c \
           string.c \
           tid.c

CPPFLAGS += -I$(includedir) -I$(includedir)/opsys

LIBRARIES = libopsys.a

include $(builddir)/targets.mk
include $(builddir)/rules.mk

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

