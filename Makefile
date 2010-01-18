
SRCDIR = src

.PHONY = all clean image

all: clean
	$(MAKE) -C $(SRCDIR) all

clean:
	$(MAKE) -C $(SRCDIR) clean

image: all
	cp res/genimage/opsyshdd.img.base opsyshdd.img
	sudo tools/genimage/genimage.sh -a i386 -b res/genimage/ -s $(SRCDIR) -i opsyshdd.img

maintainer-clean: clean
	rm -fr opsyshdd.img

