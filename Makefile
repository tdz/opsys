
.PHONY : all clean

RELOC_ADDR = 0x1000000

all : oskernel

%.S.o : %.S
	as --32 -march=i386 -o $@ $<

%.o : %.c
	gcc -m32 -march=i386 -c -o $@ $<

oskernel : boot.S.o console.o crt.o gdt.o idt.o ioports.o main.o string.o syscall.o
	ld -nostdlib -static -melf_i386 -Ttext=$(RELOC_ADDR) -o $@ $?

clean :
	rm -fr oskernel
	rm -fr *.o

