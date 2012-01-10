include $(top_srcdir)/config/utility.am

EXTRA_DIST = $(initd_SCRIPTS:=.in)

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
	sipxfreeswitch

$(initd_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,FREESWITCH_PREFIX)

DISTCLEANFILES = $(initd_SCRIPTS)
