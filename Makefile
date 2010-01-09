
KERNELDIR = kernel
APPSDIR = apps

.PHONY = all clean kernel apps

all:
	$(MAKE) -C $(KERNELDIR) all
	$(MAKE) -C $(APPSDIR) all

clean:
	$(MAKE) -C $(KERNELDIR) clean
	$(MAKE) -C $(APPSDIR) clean

