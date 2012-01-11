include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

EXTRA_DIST = $(initd_SCRIPTS:=.in)

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
	sipxivr

$(initd_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,ivr_PKGS)

DISTCLEANFILES = $(initd_SCRIPTS)
