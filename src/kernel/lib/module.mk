
LIBS += libkernel.a

libkernel.a_MODULEDIR := kernel/lib

libkernel.a_SRCS = tid.c

libkernel.a_INCLUDES += kernel/include
