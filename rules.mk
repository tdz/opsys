
# rules that apply in every makefile

DEPSDIR = .deps

.PHONY = all clean ctags

all : $(LIBRARIES) $(PROGRAMS)
	for dir in $(SUBDIRS); do ($(MAKE) -C $$dir all); done

clean :
	for dir in $(SUBDIRS); do ($(MAKE) -C $$dir clean); done
	$(RM) -fr $(DEPSDIR) $(PROGRAMS) $(LIBRARIES) $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o) $(EXTRA_CLEAN)

ctags :
	for dir in $(SUBDIRS); do ($(MAKE) -C $$dir ctags); done
	$(CTAGS) -a $(CTAGSFLAGS) *

%.a : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	$(AR) rcs $@ $^
	$(RANLIB) $@

%.S.o : %.S
	$(AS) $(ASFLAGS) -o $@ $<

%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

 $(DEPSDIR)/%.d: %.c
	@set -e; $(MKDIR) -p  $(DEPSDIR); \
		$(RM) -f $@; \
		$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
		$(SED) 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		$(RM) -f $@.$$$$

