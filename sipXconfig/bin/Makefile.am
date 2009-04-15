include $(top_srcdir)/config/subdir.am

sh_scripts = ${PACKAGE}-config

bin_SCRIPTS = \
	$(sh_scripts) \
	$(ruby_scripts)

setupbindir = @SIPX_LIBEXECDIR@/setup.d
setupbin_SCRIPTS = \
	sipxconfig-setup
setupbin_IN = $(foreach file,$(setupbin_SCRIPTS),$(file).in)

editscripts_IN = \
	sipx-mrtg-init \
	sipx-snmp-init \
	sipx-time-manager


ruby_scripts = \
	sipx-upgrade-3.6-mailstore \
	sipx-upgrade-3.6-mailstore-report

ruby_scripts_src = $(foreach f,$(ruby_scripts),$(f).rb)

TESTS = \
	$(foreach f,$(ruby_scripts),test/$(f)_test.rb)

EXTRA_DIST = \
	$(foreach file,$(editscripts_IN),$(file).in) \
	$(setupbin_IN) \
    $(sh_scripts) \
    $(ruby_scripts_src)

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

$(setupbin_SCRIPTS) : $(setupbin_IN)
	@echo "Localize $< -> $@"
	@${LocalizeSipXconfig} $(srcdir)/$(notdir $@).in > $@

$(ruby_scripts) : $(ruby_scripts_src)
	@echo "Localize $< -> $@"
	@${LocalizeSipXconfig} $(srcdir)/$(notdir $@).rb > $@

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	@echo "Create $<"
	@BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config

install-exec-hook: $(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file))

$(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file)) : $(DESTDIR)$(bindir)/% : %
	$(INSTALL) -D -m 755 $< $@

$(foreach file,$(editscripts_IN),$(file)) : % : %.in
	@echo "Localize $< -> $@"
	$(LocalizeSipXconfig) \
	-e 's,@MRTG_BINARY\@,@MRTG_BINARY@,g' \
	-e 's,@DATE_BINARY\@,@DATE_BINARY@,g' $< > $@
