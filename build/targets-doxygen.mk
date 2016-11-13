
.PHONY: doxygen

doxygen:
	$(DOXYGEN)

doc: doxygen

clean_all_files += doxygen.log
