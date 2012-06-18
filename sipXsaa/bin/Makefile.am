include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(initd_SCRIPTS:=.in)

bin_SCRIPTS = \
	sipx-snapshot-saa

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
    sipxsaa

$(initd_SCRIPTS) $(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(initd_SCRIPTS)
