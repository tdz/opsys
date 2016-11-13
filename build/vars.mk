
#
# Additional functions for make
#

strippath = $(patsubst $(abspath $1)/%,%,$(abspath $2))

#
# Computed variables
#

topdir   := .
builddir := $(call strippath,$(topdir),$(BUILDDIR))
srcdir   := $(call strippath,$(topdir),$(SRCDIR))
outdir   := $(call strippath,$(topdir),$(OUTDIR))

features := $(sort $(FEATURES))

BUILD_CPU  ?= $(shell uname -m)
HOST_CPU   ?= $(BUILD_CPU)
TARGET_CPU ?= $(HOST_CPU)

# environment variables
ENVIRONMENT ?= $(subst /,_,$(shell uname -o | tr A-Z a-z))
include $(builddir)/env-$(ENVIRONMENT).mk

# compiler toolchain
COMPILER ?= gcc
include $(builddir)/compiler-$(COMPILER).mk

# feature-specific variables
$(foreach feature,$(features),\
    $(eval include $(builddir)/vars-$(feature).mk))

.DEFAULT_GOAL := all
