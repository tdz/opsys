
ARCH = i386

CFLAGS = -m32 -I../..

ASFLAGS = --32

LDFLAGS = -nostdlib -static

RELOC_ADDR = 0x2000000

ASMSOURCES = start.S

CSOURCES = main.c

.PHONY : all clean

all : helloworld

clean :
	rm -fr helloworld
	rm -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)


helloworld : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	ld $(LDFLAGS) -melf_i386 -Ttext=$(RELOC_ADDR) -o $@ $?


%.S.o : %.S
	as $(ASFLAGS) -march=$(ARCH) -o $@ $<

%.o : %.c
	gcc $(CFLAGS) -march=$(ARCH) -c -o $@ $<

