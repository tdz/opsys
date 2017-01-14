
LIBS += libkernel.a

libkernel.a_MODULEDIR := kernel/lib

libkernel.a_SRCS = errno.c \
                   string.c \
                   tid.c

libkernel.a_INCLUDES += kernel/include
