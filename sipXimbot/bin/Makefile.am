include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

EXTRA_DIST = $(initd_SCRIPTS:=.in)

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
	sipximbot

$(initd_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,imbot_PKGS)

CLEANFILES = $(initd_SCRIPTS)
