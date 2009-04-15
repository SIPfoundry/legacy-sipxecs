include $(top_srcdir)/config/subdir.am

editscripts_IN = \
    freeswitch.sh

setupbindir = @SIPX_LIBEXECDIR@/setup.d
setupbin_SCRIPTS = \
    freeswitch_setup.sh
setupbin_IN = $(foreach file,$(setupbin_SCRIPTS),$(file).in)

EXTRA_DIST = \
   $(foreach file,$(editscripts_IN),$(file).in) \
   $(setupbin_IN)

install-exec-hook: $(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file))

$(foreach file,$(editscripts_IN),$(file)) : % : %.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

$(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file)) : $(DESTDIR)$(bindir)/% : %
	$(INSTALL) -D -m 755 $< $@

$(setupbin_SCRIPTS) : $(setupbin_IN)
	@echo "Localizing $@ -> $<"
	@${LocalizeSipXconfig} $(srcdir)/$(notdir $@).in > $@

DISTCLEANFILES = freeswitch.sh freeswitch_setup.sh
