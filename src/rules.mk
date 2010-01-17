
# rules that apply in every makefile

%.S.o : %.S
	as $(ASFLAGS) -o $@ $<

%.o : %.c
	gcc $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

