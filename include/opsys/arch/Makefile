
topdir = ../../..
include ../../../variables.mk
include ../../../flags.mk
-include ../../variables.mk
-include ../../flags.mk
-include ../variables.mk
-include ../flags.mk

CSOURCES = main.c

CPPFLAGS += -I$(includedir)/libc -I$(includedir)/opsys

LDFLAGS += -L$(srcdir)/libc/lib -L$(srcdir)/kernel/lib

LDADD := -lc -lopsys $(LDADD)

FILES = target

include ../../../rules.mk

target : FORCE
	$(RM) -f $@
	$(LN) -f -s $(target_cpu) $@

FORCE :

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

