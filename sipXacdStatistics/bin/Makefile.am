include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(setupbin_SCRIPTS:=.in) \
	$(initd_SCRIPTS:=.in)

setupbindir = @SIPX_LIBEXECDIR@
setupbin_SCRIPTS = \
	sipxacd-report-setup

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
	sipxacd-stats-daemon

bin_SCRIPTS = \
	sipxacd-stats \
	sipxacd-report

$(initd_SCRIPTS) $(setupbin_SCRIPTS) $(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(setupbin_SCRIPTS) $(bin_SCRIPTS) $(initd_SCRIPTS)
