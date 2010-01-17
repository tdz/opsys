
# some default flags needed in all makefiles

LDFLAGS += -Ttext=$(reloc_addr) -L$(libdir)/opsys

LDADD += -lopsys

