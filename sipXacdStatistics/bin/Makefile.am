include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(setupbin_SCRIPTS:=.in)

setupbindir = @SIPX_LIBEXECDIR@/setup.d
setupbin_SCRIPTS = \
	sipxconfig-report-setup

bin_SCRIPTS = \
	sipxconfig-agent \
	sipxconfig-report

$(setupbin_SCRIPTS) $(bin_SCRIPTS) : % : %.in
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(setupbin_SCRIPTS) $(bin_SCRIPTS)
