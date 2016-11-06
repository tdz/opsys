
topdir = .
include ./variables.mk
include ./flags.mk

SUBDIRS = $(includedir) $(srcdir)

EXTRA_CLEAN = doxygen.log

include ./targets.mk
include ./rules.mk

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

# include dependency rules
-include $(CSOURCES:%.c= $(DEPSDIR)/%.d)

