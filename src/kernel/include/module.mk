
.PHONY = FORCE

FILES = arch/target

arch/target : FORCE
	$(LN) -f -s $(target_cpu) $@

FORCE :
