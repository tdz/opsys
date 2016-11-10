
BINS += helloworld

helloworld_MODULEDIR := helloworld/bin

helloworld_SRCS = main.c

helloworld_INCLUDES += $(includedir)/libc $(includedir)/opsys

helloworld_LD_SEARCH_PATHS += libc/lib kernel/lib

helloworld_LDADD := -lc -lopsys

