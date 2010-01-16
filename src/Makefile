
KERNELDIR = kernel
LIBDIR = lib
APPSDIR = apps

.PHONY = all clean

all:
	$(MAKE) -C $(KERNELDIR) all
	$(MAKE) -C $(LIBDIR) all
	$(MAKE) -C $(APPSDIR) all

clean:
	$(MAKE) -C $(KERNELDIR) clean
	$(MAKE) -C $(LIBDIR) clean
	$(MAKE) -C $(APPSDIR) clean

