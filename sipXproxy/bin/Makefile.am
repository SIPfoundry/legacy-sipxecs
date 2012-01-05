include $(top_srcdir)/config/utility.am

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
    sipxproxy

EXTRA_DIST = \
	$(initd_SCRIPTS:=.in)

$(initd_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(initd_SCRIPTS)
