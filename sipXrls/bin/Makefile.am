include $(top_srcdir)/config/subdir.am

editscripts_IN = \
    sipxrls.sh \
    sipx-snapshot-rls

EXTRA_DIST = \
    $(foreach file,$(editscripts_IN),$(file).in)

install-exec-hook: $(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file))

$(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file)) : $(DESTDIR)$(bindir)/% : %
	$(INSTALL) -D -m 755 $< $@

$(foreach file,$(editscripts_IN),$(file)) : % : %.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
