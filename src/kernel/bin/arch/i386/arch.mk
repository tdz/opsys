
kernel_SRCS += $(addprefix $(archdir)/, \
        debug.c \
        gdt.c \
        idt.c \
        idt.S \
        idtentry.c \
        interupt.c \
        iomem.c \
        ioports.c \
        multiboot.c \
        multiboot.S \
        pagedir.c \
        pagetbl.c \
        pde.c \
        pte.c \
        tcbregs.c \
        tcbregs.S \
        vmem_32.c \
        vmemarea_32.c \
	)

kernel_INCLUDES += external/multiboot/include
kernel_LDSCRIPTS += $(archdir)/kernel.ld
