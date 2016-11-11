
CLEAN_ALL_FILES += tags TAGS

.PHONY: ctags

ctags:
	$(CTAGS) -a $(CTAGSFLAGS) *
