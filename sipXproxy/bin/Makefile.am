include $(top_srcdir)/config/subdir.am

GEM_LIB_DIR=$(libdir)/ruby/gems/1.8/gems
SIPX_SYSCONFDIR=$(sysconfdir)

setupbindir = @SIPX_LIBEXECDIR@/setup.d
setupbin_SCRIPTS = \
    sipxcallresolver-setup
setupbin_IN = $(foreach file,$(setupbin_SCRIPTS),$(file).in)

editscripts_IN = \
    sipXproxy.sh \
    sipxcallresolver-agent.sh \
    sipxcallresolver.sh

EXTRA_DIST = \
   $(foreach file,$(editscripts_IN),$(file).in) \
   $(setupbin_IN)

install-exec-hook: $(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file))

$(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file)) : $(DESTDIR)$(bindir)/% : %
	$(INSTALL) -D -m 755 $< $@

$(foreach file,$(editscripts_IN),$(file)) : % : %.in
	@echo "Localizing $@ -> $<"
	@$(LocalizeSipXconfig) \
		-e 's,@GEM_LIB_DIR\@,$(GEM_LIB_DIR),g' \
		-e 's,@SIPX_SYSCONFDIR\@,$(SIPX_SYSCONFDIR),g' \
		-e 's,@STUNNEL\@,@STUNNEL@,g' \
		$< > $@

$(setupbin_SCRIPTS) : $(setupbin_IN)
	@echo "Localizing $@ -> $<"
	@${LocalizeSipXconfig} $(srcdir)/$(notdir $@).in > $@
