
LIBS += libopsys.a

libopsys.a_MODULEDIR := kernel/lib

libopsys.a_SRCS = errno.c \
                  string.c \
                  tid.c

libopsys.a_INCLUDES += kernel/include
