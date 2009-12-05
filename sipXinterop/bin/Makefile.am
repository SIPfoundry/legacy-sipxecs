## Process this file with automake to produce Makefile.in
include ../config/subdir.am

include ../interop.am

bin_SCRIPTS = sipxinterop-setup \
	${PACKAGE}-config

sipxinterop-setup : $(srcdir)/sipxinterop-setup.in
	@echo "Localize $< -> $@"
	@$(INTEROP_SUBSTITUTE) $< > $@
	chmod +x $@

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	@BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config

EXTRA_DIST = sipxinterop-setup.in \
	${PACKAGE}-config

