
SRCDIR = src

EXTRA_CLEAN = doxygen.log 

.PHONY = all clean image doc html

all:
	$(MAKE) -C $(SRCDIR) all

clean:
	$(MAKE) -C $(SRCDIR) clean
	rm -fr doxygen.log

image: opsyshdd.img

maintainer-clean: clean
	rm -fr opsyshdd.img
	rm -fr doc/

doc: html

html:
	doxygen

%.img : all
	cp res/genimage/$@.base $@
	sudo tools/genimage/genimage.sh -a i386 -b res/genimage/ -s $(SRCDIR) -i $@

