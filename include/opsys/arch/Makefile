
topdir = ../../..
include ../../../variables.mk
include ../../../flags.mk
-include ../../variables.mk
-include ../../flags.mk
-include ../variables.mk
-include ../flags.mk

.PHONY = FORCE

FILES = target

include ../../../targets.mk
include ../../../rules.mk

target : FORCE
	$(LN) -f -s $(target_cpu) $@

FORCE :
