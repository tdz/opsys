
# user-space resides at 1 GiB
reloc_addr := 0x40000000

BINS += helloworld

helloworld_MODULEDIR := helloworld/bin

helloworld_SRCS = main.c

helloworld_INCLUDES += libc/include kernel/include

helloworld_LD_SEARCH_PATHS += libc/lib kernel/lib
helloworld_LDFLAGS += -Ttext=$(reloc_addr)

helloworld_LIBS += libc.a libkernel.a
