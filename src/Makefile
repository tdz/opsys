
KERNELDIR = kernel
USERDIR = user

.PHONY = all clean

all:
	$(MAKE) -C $(KERNELDIR) all
	$(MAKE) -C $(USERDIR) all

clean:
	$(MAKE) -C $(KERNELDIR) clean
	$(MAKE) -C $(USERDIR) clean

