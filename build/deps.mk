
$(DEPSDIR)/%.d: %.c
	@set -e; $(MKDIR) -p $(dir $@); \
	$(RM) -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	$(SED) 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	$(RM) -f $@.$$$$

# include dependency rules
-include $(CSOURCES:%.c=$(DEPSDIR)/%.d)
