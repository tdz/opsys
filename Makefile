
.PHONY : all clean

RELOC_ADDR = 0x1000000

all : oskernel

%.S.o : %.S
	as --32 -march=i386 -o $@ $<

oskernel : hworld.S.o
	ld -nostdlib -static -melf_i386 -Ttext=$(RELOC_ADDR) -o $@ $?

clean :
	rm -fr oskernel
	rm -fr *.S.o

