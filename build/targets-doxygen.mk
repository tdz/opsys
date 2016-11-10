
EXTRA_CLEAN += doxygen.log

.PHONY: doxygen

doxygen:
	$(DOXYGEN)

doc: doxygen
