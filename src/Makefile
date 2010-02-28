
KERNELDIR = kernel
USERDIR = user
SHAREDDIR = shared

.PHONY = all clean

all:
	$(MAKE) -C $(SHAREDDIR) all
	$(MAKE) -C $(KERNELDIR) all
	$(MAKE) -C $(USERDIR) all

clean:
	$(MAKE) -C $(SHAREDDIR) clean
	$(MAKE) -C $(KERNELDIR) clean
	$(MAKE) -C $(USERDIR) clean

