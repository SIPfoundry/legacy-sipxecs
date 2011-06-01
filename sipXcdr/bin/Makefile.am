include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

bin_SCRIPTS = \
	sipxcallresolver-agent.sh \
	sipxcallresolver.sh

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(setup_SCRIPTS:=.in)

setupdir = @SIPX_LIBEXECDIR@/setup.d
setup_SCRIPTS = \
	sipxcallresolver-setup

$(setup_SCRIPTS) $(bin_SCRIPTS) : % : %.in
	@$(call SearchAndReplace,$<,$@,STUNNEL ODBC_LIBDIR sipxcallresolver_VER)

CLEANFILES = $(bin_SCRIPTS) $(setup_SCRIPTS)
