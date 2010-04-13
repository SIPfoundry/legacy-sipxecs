include $(top_srcdir)/config/subdir.am

TESTS = sipx-upgrade-testsuite

bin_SCRIPTS = \
   ${PACKAGE}-config \
   sipx-upgrade

check_SCRIPTS = sipx-upgrade-testsuite

EXTRA_DIST = \
    $(bin_SCRIPTS) \
    sipx-upgrade-testsuite \
    shell-unit

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	@BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config
