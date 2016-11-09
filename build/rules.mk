
# rules that apply in every makefile

%.a : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	$(AR) rcs $@ $^
	$(RANLIB) $@

%.S.o : %.S
	$(AS) $(ASFLAGS) -o $@ $<

%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
