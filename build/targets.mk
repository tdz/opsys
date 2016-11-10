
#
# Per-executable variables
#

define VAR_tmpl =
$1_MODOUTDIR ?= $$(outdir)/$$($1_MODULEDIR)
$1_CPPFLAGS += $$($1_INCLUDES:%=-I%)
$1_LDFLAGS += $$($1_LD_SEARCH_PATHS:%=-L$$(outdir)/%)
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(eval $(call VAR_tmpl,$(exe))))

#
# Rules for objects that are created from C source files
#

define C_OBJ_tmpl =
$1_$2_C_SRC := $2
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
$1_$2_S_SRC := $2
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
# Directories
#

define DIR_tmpl =
$1:
	$$(MKDIR) -p $$(@D) && $$(TOUCH) $$@
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(foreach dir_,$(sort $($(exe)_DIRS)),\
        $(eval $(call DIR_tmpl,$(dir_)))))

# targets that apply in every makefile

.PHONY = all clean ctags

all : $(FILES) $(ALL_LIBS) $(ALL_BINS)
	for dir in $(SUBDIRS); do ($(MAKE) -C $$dir all); done

clean :
	for dir in $(SUBDIRS); do ($(MAKE) -C $$dir clean); done
	$(RM) -fr $(DEPSDIR) $(CLEAN_FILES) $(FILES) $(EXTRA_CLEAN)

ctags :
	for dir in $(SUBDIRS); do ($(MAKE) -C $$dir ctags); done
	$(CTAGS) -a $(CTAGSFLAGS) *
