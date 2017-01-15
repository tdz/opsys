
LIBS += libkernel.a

libkernel.a_MODULEDIR := kernel/lib

libkernel.a_SRCS += crt.c \
                    syscall.c \
                    syscall0.c

libkernel.a_INCLUDES += kernel/include libc0/include
