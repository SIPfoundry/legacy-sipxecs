include $(top_srcdir)/config/subdir.am

bin_SCRIPTS = \
	${PACKAGE}-config

EXTRA_DIST = \
    sipXvxml.sh.in \
    $(bin_SCRIPTS)

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	@BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config

install-exec-hook :
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/sipXvxml.sh.in > sipXvxml.sh
	$(INSTALL) -D -m 755 sipXvxml.sh $(DESTDIR)$(bindir)/sipXvxml.sh
