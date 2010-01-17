
# some default flags needed in all makefiles

CPPFLAGS += -I$(srcdir)/kernel

LDFLAGS += -Ttext=$(reloc_addr)

