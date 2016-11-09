
.PHONY = FORCE

FILES = target

target : FORCE
	$(LN) -f -s $(target_cpu) $@

FORCE :

