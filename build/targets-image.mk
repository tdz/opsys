
$(foreach image,$(IMAGES),\
    $(eval $(call MOD_tmpl,$(image))))

define IMG_tmpl =
$1_img := $$($1_modoutdir)/$1
$1_dir := $$(dir $$($1_img)).dir
$1_img_in := $$(IMAGE_IN_DIR)$1.base
$1_bindeps := $$(foreach bin,$$($1_BINS),$$($$(bin)_bin))
$1_deps := $$($1_bindeps)
$1_dirs := $$($1_dir)
$$($1_img): $$($1_dir) $$($1_img_in) $$($1_deps)
	$$(CP) -fr $$($1_img_in) $$@
	sudo $$(GENIMAGE) -a $$(HOST_CPU) -b $$(IMAGE_IN_DIR) -s $$(outdir) -i $$@ $$($1_deps)
clean_all_files += $$($1_img)
image_files += $$($1_img)
endef

$(foreach image,$(IMAGES),\
    $(eval $(call IMG_tmpl,$(image))))

$(foreach image,$(IMAGES),\
    $(foreach dir_,$(sort $($(image)_dirs)),\
        $(eval $(call DIR_tmpl,$(dir_)))))

.PHONY: image

image: $(image_files)
