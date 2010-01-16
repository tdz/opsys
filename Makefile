
SRCDIR = src

.PHONY = all clean

all:
	$(MAKE) -C $(SRCDIR) all

clean:
	$(MAKE) -C $(SRCDIR) clean

