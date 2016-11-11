
# some default flags needed in all makefiles

BINS =
FILES =
LIBS =

EXTRA_CLEAN = tags TAGS

# shell

SHELL = /bin/bash

# ctags

CTAGS = ctags
CTAGSFLAGS =

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
