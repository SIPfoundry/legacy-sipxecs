include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(setupbin_SCRIPTS:=.in)

setupbindir = @SIPX_LIBEXECDIR@/setup.d
setupbin_SCRIPTS = \
	sipxacd-report-setup

bin_SCRIPTS = \
	sipxacd-stats \
	sipxacd-report

$(setupbin_SCRIPTS) $(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(setupbin_SCRIPTS) $(bin_SCRIPTS)
