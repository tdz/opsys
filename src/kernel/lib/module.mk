
LIBS += libopsys.a

libopsys.a_SRCS = errno.c \
                  string.c \
                  tid.c

libopsys.a_INCLUDES += $(includedir) $(includedir)/opsys
