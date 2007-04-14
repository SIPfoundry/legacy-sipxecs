include $(top_srcdir)/config/subdir.am

sh_scripts = ${PACKAGE}-config

bin_SCRIPTS = \
	$(sh_scripts) \
	$(ruby_scripts)

ruby_scripts = \
	sipx-upgrade-3.6-mailstore \
	sipx-upgrade-3.6-mailstore-report

ruby_scripts_src = $(foreach f,$(ruby_scripts),$(f).rb)

TESTS = \
	$(foreach f,$(ruby_scripts),test/$(f)_test.rb)

EXTRA_DIST = \
    $(sh_scripts) \
    $(ruby_scripts_src)

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

$(ruby_scripts) : $(ruby_scripts_src)
	${LocalizeSipXconfig} $(srcdir)/`basename $@`.rb > $@

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config
