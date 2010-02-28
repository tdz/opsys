
LIBCSHAREDDIR = cshared
LIBOPSYSSHAREDDIR = opsysshared

.PHONY = all clean

all:
	$(MAKE) -C $(LIBCSHAREDDIR) all
	$(MAKE) -C $(LIBOPSYSSHAREDDIR) all

clean:
	$(MAKE) -C $(LIBCSHAREDDIR) clean
	$(MAKE) -C $(LIBOPSYSSHAREDDIR) clean

