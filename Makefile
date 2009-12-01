
ARCH = i386

CFLAGS = -m32

ASFLAGS = --32

LDFLAGS = -nostdlib -static

RELOC_ADDR = 0x1000000

ASMSOURCES = boot.S \
             idt.S

CSOURCES = console.c \
           crt.c \
           gdt.c \
           idt.c \
           intrrupt.c \
           ioports.c \
           main.c \
           pit.c \
           string.c \
           syscall.c

.PHONY : all clean

all : oskernel

clean :
	rm -fr oskernel
	rm -fr $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)


oskernel : $(ASMSOURCES:.S=.S.o) $(CSOURCES:.c=.o)
	ld $(LDFLAGS) -melf_i386 -Ttext=$(RELOC_ADDR) -o $@ $?


%.S.o : %.S
	as $(ASFLAGS) -march=$(ARCH) -o $@ $<

%.o : %.c
	gcc $(CFLAGS) -march=$(ARCH) -c -o $@ $<

