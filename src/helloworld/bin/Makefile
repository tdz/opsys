
topdir := ../../..
builddir := $(topdir)/build
include $(builddir)/variables.mk
include $(builddir)/flags.mk

CSOURCES = main.c

CPPFLAGS += -I$(includedir)/libc -I$(includedir)/opsys

LDFLAGS += -L$(srcdir)/libc/lib -L$(srcdir)/kernel/lib

LDADD := -lc -lopsys $(LDADD)

PROGRAMS = helloworld

include $(builddir)/targets.mk
include $(builddir)/rules.mk

helloworld : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDADD)

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

