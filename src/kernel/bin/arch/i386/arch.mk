
oskernel_SRCS += $(addprefix $(archdir)/, \
        debug.c \
        gdt.c \
        idt.c \
        idt.S \
        idtentry.c \
        interupt.c \
        ioports.c \
        multiboot.c \
        multiboot.S \
        pagedir.c \
        pagetbl.c \
        pde.c \
        pte.c \
        tcbregs.c \
        tcbregs.S \
        vmem.c \
        vmem_32.c \
        vmem_pae.c \
        vmemhlp.c \
	)

oskernel_INCLUDES += external/multiboot/include
oskernel_LDSCRIPTS += $(archdir)/oskernel.ld
