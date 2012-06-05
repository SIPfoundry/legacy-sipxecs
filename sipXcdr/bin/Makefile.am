include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(initd_SCRIPTS:=.in) \
	$(bin_SCRIPTS:=.in)

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
	sipxcdr

bindir = @SIPX_BINDIR@
bin_SCRIPTS = \
	sipxcdr-archive

$(bin_SCRIPTS) $(initd_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,ODBC_LIBDIR sipxcallresolver_VER)

CLEANFILES = $(initd_SCRIPTS) $(bin_SCRIPTS)
