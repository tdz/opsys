
# rules that apply in every makefile

%.S.o : %.S
	as $(ASFLAGS) -o $@ $<

%.o : %.c
	gcc $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.d: %.c
	@set -e; rm -f $@; \
		$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$

