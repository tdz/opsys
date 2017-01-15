
LIBS += libc0.a

libc0.a_MODULEDIR := libc0/lib

libc0.a_SRCS = errno.c \
               string.c

libc0.a_INCLUDES += libc0/include
