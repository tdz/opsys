
features := $(sort $(FEATURES))

# some default flags needed in all makefiles

BINS =
FILES =
LIBS =

# shell

SHELL = /bin/bash

# others

CP = cp
LN = ln
MKDIR = mkdir
RM = rm
SED = sed
TOUCH := touch

# compiler toolchain
COMPILER ?= gcc
include $(builddir)/compiler-$(COMPILER).mk

# feature-specific variables
$(foreach feature,$(features),\
    $(eval include $(builddir)/vars-$(feature).mk))
