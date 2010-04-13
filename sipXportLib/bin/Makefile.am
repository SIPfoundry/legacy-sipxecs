include $(top_srcdir)/config/subdir.am

TESTS = sipx-upgrade-testsuite

bin_SCRIPTS = \
   ${PACKAGE}-config \
   sipx-config \
	sipx-upgrade

check_SCRIPTS = sipx-upgrade-testsuite

EXTRA_DIST = \
    $(bin_SCRIPTS) \
    sipx-upgrade-testsuite.in \
    shell-unit.in \
    sipx-upgrade.in

install-exec-hook :

sipx-upgrade: $(srcdir)/sipx-upgrade.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/sipx-upgrade.in > sipx-upgrade

shell-unit: $(srcdir)/shell-unit.in
	cp $(srcdir)/shell-unit.in shell-unit

$(DESTDIR)$(bindir)/sipx-upgrade: sipx-upgrade
	$(INSTALL) -D -m 755 $(srcdir)/sipx-upgrade $(DESTDIR)$(bindir)/sipx-upgrade

sipx-upgrade-testsuite: sipx-upgrade shell-unit $(srcdir)/sipx-upgrade-testsuite.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/sipx-upgrade-testsuite.in > $@
	chmod +x $@

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	@BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config

DISTCLEANFILES = Makefile.in sipxportlib-config config.h.in
