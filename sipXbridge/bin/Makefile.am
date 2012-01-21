include $(top_srcdir)/config/utility.am

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
    sipxbridge

EXTRA_DIST = \
	$(initd_SCRIPTS:=.in)

$(initd_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,bridge_PKGS)

CLEANFILES = $(initd_SCRIPTS)
