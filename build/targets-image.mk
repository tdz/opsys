
IMAGES := opsyshdd.img

.PHONY: image

image: $(IMAGES)

%.img : all
	$(CP) res/genimage/$@.base $@
	sudo $(GENIMAGE) -a $(HOST_CPU) -b res/genimage/ -s $(outdir) -i $@

CLEAN_ALL_FILES += $(IMAGES)
