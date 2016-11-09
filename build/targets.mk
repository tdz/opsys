
#
# Per-executable variables
#

define VAR_tmpl =
$1_CPPFLAGS += $$($1_INCLUDES:%=-I%)
$1_LDFLAGS += $$($1_LD_SEARCH_PATHS:%=-L%)
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(eval $(call VAR_tmpl,$(exe))))

#
# Rules for objects that are created from C source files
#

define C_OBJ_tmpl =
$2.o: $2
	$$(CC) $$($1_CPPFLAGS) $$(CPPFLAGS) $$($1_CFLAGS) $$(CFLAGS) -c -o $$@ $2
$1_C_SRCS += $2
$1_C_OBJS += $2.o
CLEAN_FILES += $2.o
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(foreach src,$(filter %.c,$($(exe)_SRCS)),\
	    $(eval $(call C_OBJ_tmpl,$(exe),$(src)))))

#
# Rules for objects that are created from assembly source files
#

define S_OBJ_tmpl =
$2.o: $2
	$$(AS) $$($1_ASFLAGS) $$(ASFLAGS) -o $$@ $2
$1_S_SRCS += $2
$1_S_OBJS += $2.o
CLEAN_FILES += $2.o
endef

$(foreach exe,$(LIBS) $(BINS),\
    $(foreach src,$(filter %.S,$($(exe)_SRCS)),\
	    $(eval $(call S_OBJ_tmpl,$(exe),$(src)))))

#
# Rules for libraries
#

define LIB_tmpl =
$1_OBJS += $$($1_C_OBJS) $$($1_S_OBJS)
$1: $$($1_OBJS)
	$$(AR) rcs $$@ $$($1_OBJS)
	$$(RANLIB) $$@
CLEAN_FILES += $1
endef

$(foreach lib,$(LIBS),\
    $(eval $(call LIB_tmpl,$(lib))))

#
# Rules for binaries
#

define BIN_tmpl =
$1_OBJS += $$($1_C_OBJS) $$($1_S_OBJS)
$1: $$($1_OBJS)
	$$(LD) $$($1_LDFLAGS) $$(LDFLAGS) -o $$@ $$($1_OBJS) $$($1_LDADD) $$(LDADD)
CLEAN_FILES += $1
endef

$(foreach bin,$(BINS),\
    $(eval $(call BIN_tmpl,$(bin))))

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
