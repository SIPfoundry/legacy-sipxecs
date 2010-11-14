include $(top_srcdir)/config/subdir.am

bin_SCRIPTS = \
        ${PACKAGE}-config

editscripts_IN = \
    openacd.sh

setupbindir = @SIPX_LIBEXECDIR@/setup.d
setupbin_SCRIPTS = \
    openacd_setup.sh
setupbin_IN = $(foreach file,$(setupbin_SCRIPTS),$(file).in)

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

$(foreach file,$(editscripts_IN),$(file)) : % : %.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

$(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file)) : $(DESTDIR)$(bindir)/% : %
	$(INSTALL) -D -m 755 $< $@

$(setupbin_SCRIPTS) : $(setupbin_IN)
	@echo "Localizing $@ -> $<"
	@${LocalizeSipXconfig} $(srcdir)/$(notdir $@).in > $@

DISTCLEANFILES = openacd.sh openacd_setup.sh
