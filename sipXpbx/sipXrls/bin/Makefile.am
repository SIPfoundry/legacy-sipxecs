include $(top_srcdir)/config/subdir.am

bin_SCRIPTS = \
	sipxrls-config

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

sipxrls-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	@BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > sipxrls-config
	chmod +x sipxrls-config

editscripts_IN = \
    sipxrls.sh \
    sipx-snapshot-rls

EXTRA_DIST = \
    $(foreach file,$(editscripts_IN),$(file).in) \
    $(bin_SCRIPTS)

install-exec-hook: $(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file))

$(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file)) : $(DESTDIR)$(bindir)/% : %
	$(INSTALL) -D -m 755 $< $@

$(foreach file,$(editscripts_IN),$(file)) : % : %.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
