
topdir := .
builddir := $(topdir)/build
include $(builddir)/variables.mk
include $(builddir)/flags.mk

.DEFAULT_GOAL := all

SUBDIRS = $(includedir) $(srcdir)

EXTRA_CLEAN = doxygen.log

IMAGES = opsyshdd.img

.PHONY += image doc html

image: $(IMAGES)

maintainer-clean: clean
	$(RM) -fr doc/ $(IMAGES)

doc: html

html:
	doxygen

%.img : all
	$(CP) res/genimage/$@.base $@
	sudo tools/genimage/genimage.sh -a $(target_cpu) -b res/genimage/ -s $(outdir) -i $@

include $(builddir)/deps.mk
include $(builddir)/targets.mk
