
# some default flags needed in all makefiles

LDFLAGS += -Ttext=$(reloc_addr) -L$(srcdir)/shared/lib/cshared -L$(libdir)/opsys -e_start

LDADD += -lcshared -lopsys

