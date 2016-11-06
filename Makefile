
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
	sudo tools/genimage/genimage.sh -a $(target_cpu) -b res/genimage/ -s $(srcdir) -i $@

include $(builddir)/deps.mk
include $(builddir)/rules.mk
include $(builddir)/targets.mk
