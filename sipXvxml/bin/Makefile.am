include $(top_srcdir)/config/subdir.am

bin_SCRIPTS = \
	${PACKAGE}-config

EXTRA_DIST = \
    sipXvxml.sh.in \
    sipXvxml_setup.sh.in \
    $(bin_SCRIPTS)

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	@BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config

install-exec-hook : \
	$(DESTDIR)$(bindir)/sipXvxml.sh \
	$(DESTDIR)$(libexecdir)/sipXecs/setup.d/sipXvxml_setup.sh

$(DESTDIR)$(bindir)/sipXvxml.sh : sipXvxml.sh
	$(INSTALL) -D -m 755 $< $@

sipXvxml.sh : sipXvxml.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

$(DESTDIR)$(libexecdir)/sipXecs/setup.d/sipXvxml_setup.sh : sipXvxml.sh
	$(INSTALL) -D -m 755 $< $@

sipXvxml.sh : sipXvxml.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
