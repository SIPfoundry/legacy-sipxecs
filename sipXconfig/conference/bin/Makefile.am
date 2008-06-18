include $(top_srcdir)/config/subdir.am

editscripts_IN = \
    freeswitch.sh

EXTRA_DIST = \
   $(foreach file,$(editscripts_IN),$(file).in)

install-exec-hook: $(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file))

$(foreach file,$(editscripts_IN),$(file)) : % : %.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

$(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file)) : $(DESTDIR)$(bindir)/% : %
	$(INSTALL) -D -m 755 $< $@

DISTCLEANFILES = freeswitch.sh
