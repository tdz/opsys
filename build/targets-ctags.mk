
.PHONY: ctags

ctags:
	$(CTAGS) -a $(CTAGSFLAGS) *

clean_all_files += tags TAGS
