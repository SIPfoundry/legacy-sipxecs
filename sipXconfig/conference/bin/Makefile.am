include $(top_srcdir)/config/subdir.am

editscripts_IN = \
    freeswitch.sh

EXTRA_DIST = \
    $(editscripts_IN)

install-exec-hook: $(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file))

$(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file)) : $(DESTDIR)$(bindir)/% : %
	$(INSTALL) -D -m 755 $< $@

$(foreach file,$(editscripts_IN),$(file)) : % : %.in
	$(LocalizeSipXconfig) $< > $@

