
$(foreach image,$(IMAGES),\
    $(eval $(call MOD_tmpl,$(image))))

define IMG_tmpl =
$1_IMG := $$($1_modoutdir)/$1
$1_DIR := $$(dir $$($1_IMG)).dir
$1_IMG_IN := $$(IMAGE_IN_DIR)$1.base
$1_BINDEPS += $$(foreach bin,$$($1_BINS),$$($$(bin)_BIN))
$1_DEPS += $$($1_BINDEPS)
$$($1_IMG): $$($1_DIR) $$($1_IMG_IN) $$($1_DEPS)
	$$(CP) -fr $$($1_IMG_IN) $$@
	sudo $$(GENIMAGE) -a $$(HOST_CPU) -b $$(IMAGE_IN_DIR) -s $$(outdir) -i $$@ $$($1_DEPS)
$1_DIRS += $$($1_DIR)
CLEAN_ALL_FILES += $$($1_IMG)
IMAGE_FILES += $$($1_IMG)
endef

$(foreach image,$(IMAGES),\
    $(eval $(call IMG_tmpl,$(image))))

$(foreach image,$(IMAGES),\
    $(foreach dir_,$(sort $($(image)_DIRS)),\
        $(eval $(call DIR_tmpl,$(dir_)))))

.PHONY: image

image: $(IMAGE_FILES)
