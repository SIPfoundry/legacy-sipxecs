include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

EXTRA_DIST = $(initd_SCRIPTS:=.in)

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
	sipxacccode

$(initd_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,sipxacc_PKGS)

CLEANFILES = $(initd_SCRIPTS)
