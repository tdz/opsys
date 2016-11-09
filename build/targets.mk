
# targets that apply in every makefile

.PHONY = all clean ctags

all : $(FILES) $(LIBRARIES) $(PROGRAMS)
	for dir in $(SUBDIRS); do ($(MAKE) -C $$dir all); done

clean :
	for dir in $(SUBDIRS); do ($(MAKE) -C $$dir clean); done
	$(RM) -fr $(DEPSDIR) $(PROGRAMS) $(LIBRARIES) $(FILES) $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o) $(EXTRA_CLEAN)

ctags :
	for dir in $(SUBDIRS); do ($(MAKE) -C $$dir ctags); done
	$(CTAGS) -a $(CTAGSFLAGS) *