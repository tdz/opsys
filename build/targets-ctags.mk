
EXTRA_CLEAN += tags TAGS

.PHONY: ctags

ctags:
	$(CTAGS) -a $(CTAGSFLAGS) *
