include $(top_srcdir)/config/subdir.am

GEM_LIB_DIR=$(libdir)/ruby/gems/1.8/gems
SIPX_SYSCONFDIR=$(sysconfdir)

bin_SCRIPTS = \
	${PACKAGE}-config

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
   $(setupbin_IN) \
   $(bin_SCRIPTS)

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	@echo "making ${PACKAGE}-config"
	@BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config

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
