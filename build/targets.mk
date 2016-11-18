
#
# Per-module variables
#

define MOD_tmpl =
$1_modsrcdir := $$(call strippath,$$(topdir)/,$$(srcdir)/$$($1_MODULEDIR)/)/
$1_modoutdir := $$(call strippath,$$(topdir)/,$$(outdir)/$$($1_MODULEDIR)/)/
endef

$(foreach mod,$(SYMLINKS) $(LIBS) $(BINS),\
    $(eval $(call MOD_tmpl,$(mod))))

#
# Per-executable variables
#

define EXE_tmpl =
$1_cppflags := $$(CPPFLAGS) $$($1_CPPFLAGS) $$($1_INCLUDES:%=-I$$(srcdir)/%)
$1_cflags   := $$(CFLAGS) $$($1_CFLAGS)
$1_asflags  := $$(ASFLAGS) $$($1_ASFLAGS)
$1_ldflags  := $$(LDFLAGS) $$($1_LDFLAGS) $$($1_LD_SEARCH_PATHS:%=-L$$(outdir)/%)
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(eval $(call EXE_tmpl,$(exe))))

#
# Rules for objects that are created from C source files
#

define C_OBJ_tmpl =
$1_$2_c_src := $$($1_modsrcdir)$2
$1_$2_c_obj := $$($1_modoutdir)$2.o
$1_$2_c_dir := $$(dir $$($1_$2_c_obj)).dir
$$($1_$2_c_obj): $$($1_$2_c_dir) $$($1_$2_c_src)
	$$(CC) $$($1_cppflags) $$($1_cflags) -c -o $$@ $$($1_$2_c_src)
$1_c_srcs += $$($1_$2_c_src)
$1_c_objs += $$($1_$2_c_obj)
$1_c_dirs += $$($1_$2_c_dir)
clean_files += $$($1_$2_c_obj)
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(foreach src,$(filter %.c,$($(exe)_SRCS)),\
	    $(eval $(call C_OBJ_tmpl,$(exe),$(src)))))

#
# Rules for objects that are created from assembly source files
#

define S_OBJ_tmpl =
$1_$2_s_src := $$($1_modsrcdir)$2
$1_$2_s_obj := $$($1_modoutdir)$2.o
$1_$2_s_dir := $$(dir $$($1_$2_s_obj)).dir
$$($1_$2_s_obj): $$($1_$2_s_dir) $$($1_$2_s_src)
	$$(AS) $$($1_asflags) -o $$@ $$($1_$2_s_src)
$1_s_srcs += $$($1_$2_s_src)
$1_s_objs += $$($1_$2_s_obj)
$1_s_dirs += $$($1_$2_s_dir)
clean_files += $$($1_$2_s_obj)
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(foreach src,$(filter %.S,$($(exe)_SRCS)),\
	    $(eval $(call S_OBJ_tmpl,$(exe),$(src)))))

#
# Rules for libraries
#

define LIB_tmpl =
$1_lib := $$($1_modoutdir)$1
$1_dir := $$(dir $$($1_lib)).dir
$1_objs := $$($1_c_objs) $$($1_s_objs)
$1_dirs := $$($1_dir) $$($1_c_dirs) $$($1_s_dirs)
$$($1_lib): $$($1_dir) $$($1_objs)
	$$(AR) rcs $$@ $$($1_objs)
	$$(RANLIB) $$@
clean_files += $$($1_lib)
all_libs += $$($1_lib)
endef

$(foreach lib,$(LIBS),\
    $(eval $(call LIB_tmpl,$(lib))))

#
# Rules for binaries
#

define BIN_tmpl =
$1_bin := $$($1_modoutdir)$1
$1_dir := $$(dir $$($1_bin)).dir
$1_objs := $$($1_c_objs) $$($1_s_objs)
$1_dirs := $$($1_dir) $$($1_c_dirs) $$($1_s_dirs)
$1_ldscripts := $$(foreach ldscript,$$($1_LDSCRIPTS), -T$$($1_modsrcdir)$$(ldscript))
$1_ldadd := $$(LDADD) $$(patsubst lib%.a,-l%,$$($1_LIBS))
$1_libs := $$(foreach lib,$$($1_LIBS),$$($$(lib)_lib))
$$($1_bin): $$($1_dir) $$($1_objs) $$($1_libs)
	$$(LD) $$($1_ldflags) $$($1_ldscripts) -o $$@ $$($1_objs) $$($1_ldadd)
clean_files += $$($1_bin)
all_bins += $$($1_bin)
endef

$(foreach bin,$(BINS),\
    $(eval $(call BIN_tmpl,$(bin))))

#
# Rules for links
#

define SYMLINK_tmpl
$1_symlink := $$($1_modoutdir)$1
$1_dst := $$($1_modsrcdir)$$($1_TARGET)
$1_dir := $$(dir $$($1_symlink)).dir
$$($1_symlink): $$($1_dir) $$($1_dst)
	$$(LN) -fs $$($1_dst) $$@
$1_dirs += $$($1_dir)
clean_files += $$($1_symlink)
all_symlinks += $$($1_symlink)
endef

$(foreach symlink,$(SYMLINKS),\
    $(eval $(call SYMLINK_tmpl,$(symlink))))

#
# Directories
#

define DIR_tmpl =
$1:
	$$(MKDIR) -p $$(@D) && $$(TOUCH) $$@
clean_all_files += $1
endef

$(foreach exe,$(SYMLINKS) $(LIBS) $(BINS),\
    $(foreach dir_,$(sort $($(exe)_dirs)),\
        $(eval $(call DIR_tmpl,$(dir_)))))

#
# Dependencies
#

define C_DEP_tmpl =
$1_$2_c_dep := $$($1_modoutdir)$2.d
$$($1_$2_c_dep): $$($1_$2_c_dir) $$($1_$2_c_src)
	$$(CC) -M -MT '$$($1_$2_c_obj) $$@ ' $$($1_cppflags) $$($1_$2_c_src) > $$@;
clean_files += $$($1_$2_c_dep)
-include $$($1_$2_c_dep)
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(foreach src,$(filter %.c,$($(exe)_SRCS)),\
	    $(eval $(call C_DEP_tmpl,$(exe),$(src)))))

#
# Feature-specific targets
#

$(foreach feature,$(features),\
    $(eval include $(builddir)/targets-$(feature).mk))

#
# Common targets
#

.PHONY: all clean clean-all doc install

all: | $(FILES) $(all_symlinks) $(all_bins)

clean:
	$(RM) -fr $(clean_files) $(FILES)

clean-all: clean
	$(RM) -fr $(clean_all_files)

doc:

install: $(install_targets)
