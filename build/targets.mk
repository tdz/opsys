
#
# Per-module variables
#

define MOD_tmpl =
$1_MODSRCDIR ?= $$(abspath $$(srcdir)/$$($1_MODULEDIR))
$1_MODOUTDIR ?= $$(abspath $$(outdir)/$$($1_MODULEDIR))
endef

$(foreach mod,$(SYMLINKS) $(LIBS) $(BINS),\
    $(eval $(call MOD_tmpl,$(mod))))

#
# Per-executable variables
#

define EXE_tmpl =
$1_CPPFLAGS += $$($1_INCLUDES:%=-I$$(srcdir)/%)
$1_LDFLAGS += $$($1_LD_SEARCH_PATHS:%=-L$$(outdir)/%)
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(eval $(call EXE_tmpl,$(exe))))

#
# Rules for objects that are created from C source files
#

define C_OBJ_tmpl =
$1_$2_C_SRC := $$($1_MODSRCDIR)/$2
$1_$2_C_OBJ := $$($1_MODOUTDIR)/$2.o
$1_$2_C_DIR := $$(dir $$($1_$2_C_OBJ)).dir
$$($1_$2_C_OBJ): $$($1_$2_C_DIR) $$($1_$2_C_SRC)
	$$(CC) $$($1_CPPFLAGS) $$(CPPFLAGS) $$($1_CFLAGS) $$(CFLAGS) -c -o $$@ $$($1_$2_C_SRC)
$1_C_SRCS += $$($1_$2_C_SRC)
$1_C_OBJS += $$($1_$2_C_OBJ)
$1_C_DIRS += $$($1_$2_C_DIR)
CLEAN_FILES += $$($1_$2_C_OBJ)
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(foreach src,$(filter %.c,$($(exe)_SRCS)),\
	    $(eval $(call C_OBJ_tmpl,$(exe),$(src)))))

#
# Rules for objects that are created from assembly source files
#

define S_OBJ_tmpl =
$1_$2_S_SRC := $$($1_MODSRCDIR)/$2
$1_$2_S_OBJ := $$($1_MODOUTDIR)/$2.o
$1_$2_S_DIR := $$(dir $$($1_$2_S_OBJ)).dir
$$($1_$2_S_OBJ): $$($1_$2_S_DIR) $$($1_$2_S_SRC)
	$$(AS) $$($1_ASFLAGS) $$(ASFLAGS) -o $$@ $$($1_$2_S_SRC)
$1_S_SRCS += $$($1_$2_S_SRC)
$1_S_OBJS += $$($1_$2_S_OBJ)
$1_S_DIRS += $$($1_$2_S_DIR)
CLEAN_FILES += $$($1_$2_S_OBJ)
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(foreach src,$(filter %.S,$($(exe)_SRCS)),\
	    $(eval $(call S_OBJ_tmpl,$(exe),$(src)))))

#
# Rules for libraries
#

define LIB_tmpl =
$1_LIB := $$($1_MODOUTDIR)/$1
$1_DIR := $$(dir $$($1_LIB)).dir
$1_OBJS += $$($1_C_OBJS) $$($1_S_OBJS)
$1_DIRS += $$($1_DIR) $$($1_C_DIRS) $$($1_S_DIRS)
$$($1_LIB): $$($1_DIR) $$($1_OBJS)
	$$(AR) rcs $$@ $$($1_OBJS)
	$$(RANLIB) $$@
CLEAN_FILES += $$($1_LIB)
ALL_LIBS += $$($1_LIB)
endef

$(foreach lib,$(LIBS),\
    $(eval $(call LIB_tmpl,$(lib))))

#
# Rules for binaries
#

define BIN_tmpl =
$1_BIN := $$($1_MODOUTDIR)/$1
$1_DIR := $$(dir $$($1_BIN)).dir
$1_OBJS += $$($1_C_OBJS) $$($1_S_OBJS)
$1_DIRS += $$($1_DIR) $$($1_C_DIRS) $$($1_S_DIRS)
$$($1_BIN): $$($1_DIR) $$($1_OBJS)
	$$(LD) $$($1_LDFLAGS) $$(LDFLAGS) -o $$@ $$($1_OBJS) $$($1_LDADD) $$(LDADD)
CLEAN_FILES += $$($1_BIN)
ALL_BINS += $$($1_BIN)
endef

$(foreach bin,$(BINS),\
    $(eval $(call BIN_tmpl,$(bin))))

#
# Rules for links
#

define SYMLINK_tmpl
$1_symlink := $$($1_MODOUTDIR)/$1
$1_dst := $$($1_MODSRCDIR)/$$($1_TARGET)
$1_dir := $$(dir $$($1_symlink)).dir
$$($1_symlink): $$($1_dir) $$($1_dst)
	$$(LN) -fs $$($1_dst) $$@
$1_DIRS += $$($1_dir)
CLEAN_FILES += $$($1_symlink)
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
endef

$(foreach exe,$(SYMLINKS) $(LIBS) $(BINS),\
    $(foreach dir_,$(sort $($(exe)_DIRS)),\
        $(eval $(call DIR_tmpl,$(dir_)))))

# targets that apply in every makefile

.PHONY: all clean ctags

all: | $(FILES) $(all_symlinks) $(ALL_LIBS) $(ALL_BINS)

clean:
	$(RM) -fr $(DEPSDIR) $(CLEAN_FILES) $(FILES) $(EXTRA_CLEAN)

ctags:
	$(CTAGS) -a $(CTAGSFLAGS) *
