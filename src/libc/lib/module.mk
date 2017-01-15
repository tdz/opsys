
LIBS += libc.a

libc.a_MODULEDIR := libc/lib

libc.a_SRCS = start.S

libc.a_INCLUDES += libc/include kernel/include libc0/include
