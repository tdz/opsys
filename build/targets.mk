
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
$$($1_MODOUTDIR)/$2.o: $$(dir $$($1_MODOUTDIR)/$2.o)/.dir $2
	$$(CC) $$($1_CPPFLAGS) $$(CPPFLAGS) $$($1_CFLAGS) $$(CFLAGS) -c -o $$@ $2
$1_C_SRCS += $2
$1_C_OBJS += $$($1_MODOUTDIR)/$2.o
DIRS += $$(dir $$($1_MODOUTDIR)/$2.o)
CLEAN_FILES += $$($1_MODOUTDIR)/$2.o
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(foreach src,$(filter %.c,$($(exe)_SRCS)),\
	    $(eval $(call C_OBJ_tmpl,$(exe),$(src)))))

#
# Rules for objects that are created from assembly source files
#

define S_OBJ_tmpl =
$$($1_MODOUTDIR)/$2.o: $$(dir $$($1_MODOUTDIR)/$2.o)/.dir $2
	$$(AS) $$($1_ASFLAGS) $$(ASFLAGS) -o $$@ $2
$1_S_SRCS += $2
$1_S_OBJS += $$($1_MODOUTDIR)/$2.o
DIRS += $$(dir $$($1_MODOUTDIR)/$2.o)
CLEAN_FILES += $$($1_MODOUTDIR)/$2.o
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(foreach src,$(filter %.S,$($(exe)_SRCS)),\
	    $(eval $(call S_OBJ_tmpl,$(exe),$(src)))))

#
# Rules for libraries
#

define LIB_tmpl =
$1_OBJS += $$($1_C_OBJS) $$($1_S_OBJS)
$$($1_MODOUTDIR)/$1: $$(dir $$($1_MODOUTDIR)/$1)/.dir $$($1_OBJS)
	$$(AR) rcs $$@ $$($1_OBJS)
	$$(RANLIB) $$@
DIRS += $$(dir $$($1_MODOUTDIR)/$1)
CLEAN_FILES += $$($1_MODOUTDIR)/$1
ALL_LIBS += $$($1_MODOUTDIR)/$1
endef

$(foreach lib,$(LIBS),\
    $(eval $(call LIB_tmpl,$(lib))))

#
# Rules for binaries
#

define BIN_tmpl =
$1_OBJS += $$($1_C_OBJS) $$($1_S_OBJS)
$$($1_MODOUTDIR)/$1: $$(dir $$($1_MODOUTDIR)/$1)/.dir $$($1_OBJS)
	$$(LD) $$($1_LDFLAGS) $$(LDFLAGS) -o $$@ $$($1_OBJS) $$($1_LDADD) $$(LDADD)
DIRS += $$(dir $$($1_MODOUTDIR)/$1)
CLEAN_FILES += $$($1_MODOUTDIR)/$1
ALL_BINS += $$($1_MODOUTDIR)/$1
endef

$(foreach bin,$(BINS),\
    $(eval $(call BIN_tmpl,$(bin))))

#
# Directories
#

define DIR_tmpl =
$1/.dir:
	$$(MKDIR) -p $$(@D) && $$(TOUCH) $$@
endef

$(foreach dir_,$(sort $(DIRS)),\
    $(eval $(call DIR_tmpl,$(dir_))))

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
