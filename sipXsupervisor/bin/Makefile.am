include $(top_srcdir)/config/subdir.am

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	@BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config

bin_SCRIPTS = \
	sipxsupervisor.sh \
	${PACKAGE}-config

sipxsupervisor.sh: sipxsupervisor.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

EXTRA_DIST = sipxsupervisor.sh.in


