
topdir := .
builddir := $(topdir)/build
include $(builddir)/variables.mk
include $(builddir)/flags.mk

.DEFAULT_GOAL := all

# Keep sorted until dependencies are tracked correctly
include $(srcdir)/kernel/include/module.mk
include $(srcdir)/kernel/lib/module.mk
include $(srcdir)/kernel/bin/module.mk
include $(srcdir)/libc/lib/module.mk
include $(srcdir)/helloworld/bin/module.mk

EXTRA_CLEAN = doxygen.log

IMAGES = opsyshdd.img

.PHONY: image doc html

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
