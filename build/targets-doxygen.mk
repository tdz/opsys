
CLEAN_ALL_FILES += doxygen.log

.PHONY: doxygen

doxygen:
	$(DOXYGEN)

doc: doxygen
