
ARCH = i386

CFLAGS = -m32 -Wall -ansi -g3

ASFLAGS = --32

LDFLAGS = -nostdlib -static

RELOC_ADDR = 0x100000

ASMSOURCES = idt.S \
             multiboot.S

CSOURCES = console.c \
           crt.c \
           elfldr.c \
           gdt.c \
           idt.c \
           intrrupt.c \
           ioports.c \
           multiboot.c \
           physmem.c \
           pde.c \
           pit.c \
           pte.c \
           string.c \
           syscall.c \
           task.c \
           tcb.c \
           virtmem.c

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

