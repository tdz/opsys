
LIBS += libc.a

libc.a_MODULEDIR := libc/lib

libc.a_SRCS = start.S \
              syscall.c \
              syscall0.c \
              write.c

libc.a_INCLUDES += $(includedir)/libc $(includedir)/opsys
