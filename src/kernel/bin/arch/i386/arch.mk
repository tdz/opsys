
oskernel_ASMSOURCES += $(addprefix $(archdir)/, \
        idt.S \
        multiboot.S \
        tcbregs.S \
	)

oskernel_CSOURCES += $(addprefix $(archdir)/, \
        debug.c \
        gdt.c \
        idt.c \
        idtentry.c \
        interupt.c \
        ioports.c \
        main.c \
        multiboot.c \
        pagedir.c \
        pagetbl.c \
        pde.c \
        pic.c \
        pte.c \
        tcb.c \
        tcbregs.c \
        vmem.c \
        vmem_32.c \
        vmem_pae.c \
        vmemhlp.c \
	)

# kernel resides at 1 MiB
reloc_addr = 0x100000

oskernel_LDFLAGS += -Ttext=$(reloc_addr) -e_start
