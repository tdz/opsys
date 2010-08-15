
# some default flags needed in all makefiles

LDFLAGS += -Ttext=$(reloc_addr) -L$(srcdir)/shared/lib/cshared -L$(srcdir)/shared/lib/opsysshared -L$(libdir)/opsys -e_start

LDADD := -lcshared -lopsysshared -lopsys $(LDADD)

