
#
# Per-executable variables
#

define VAR_tmpl =
$1_CPPFLAGS += $$($1_INCLUDES:%=-I%)
$1_LDFLAGS += $$($1_LD_SEARCH_PATHS:%=-L%)
$1_C_OBJS += $$($1_CSOURCES:%.c=%.c.o)
$1_S_OBJS += $$($1_ASMSOURCES:%.S=%.S.o)
$1_OBJS += $$($1_C_OBJS) $$($1_S_OBJS)
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(eval $(call VAR_tmpl,$(exe))))

#
# Rules for libraries
#

define LIB_tmpl =
CLEAN_FILES += $1
$1: $$($1_OBJS)
	$$(AR) rcs $$@ $$($1_OBJS)
	$$(RANLIB) $$@
endef

$(foreach lib,$(LIBS),\
    $(eval $(call LIB_tmpl,$(lib))))

#
# Rules for binaries
#

define BIN_tmpl =
CLEAN_FILES += $1
$1: $$($1_OBJS)
	$$(LD) $$($1_LDFLAGS) $$(LDFLAGS) -o $$@ $$($1_OBJS) $$($1_LDADD) $$(LDADD)
endef

$(foreach bin,$(BINS),\
    $(eval $(call BIN_tmpl,$(bin))))

#
# Rules for objects that are created from C source files
#

define C_OBJ_tmpl =
CLEAN_FILES += $1
$1: $$(basename $1)
	$$(CC) $$($2_CPPFLAGS) $$(CPPFLAGS) $$($2_CFLAGS) $$(CFLAGS) -c -o $$@ $$<
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(foreach obj,$($(exe)_C_OBJS),\
	    $(eval $(call C_OBJ_tmpl,$(obj),$(exe)))))

#
# Rules for objects that are created from assembly source files
#

define S_OBJ_tmpl =
CLEAN_FILES += $1
$1: $$(basename $1)
	$$(AS) $$($2_ASFLAGS) $$(ASFLAGS) -o $$@ $$<
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(foreach obj,$($(exe)_S_OBJS),\
	    $(eval $(call S_OBJ_tmpl,$(obj),$(exe)))))

# targets that apply in every makefile

.PHONY = all clean ctags

all : $(FILES) $(LIBS) $(BINS)
	for dir in $(SUBDIRS); do ($(MAKE) -C $$dir all); done

clean :
	for dir in $(SUBDIRS); do ($(MAKE) -C $$dir clean); done
	$(RM) -fr $(DEPSDIR) $(CLEAN_FILES) $(FILES) $(EXTRA_CLEAN)

ctags :
	for dir in $(SUBDIRS); do ($(MAKE) -C $$dir ctags); done
	$(CTAGS) -a $(CTAGSFLAGS) *
