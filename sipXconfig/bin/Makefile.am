include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(setup_SCRIPTS:=.in) \
	$(initd_SCRIPTS:=.in) \
	$(libexec_SCRIPTS:=.in)

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
	sipxconfig

bin_SCRIPTS = \
	sipx-time-manager

libexecdir = @SIPX_LIBEXECDIR@
libexec_SCRIPTS = \
  sipxconfig-archive

$(libexec_SCRIPTS) $(initd_SCRIPTS) $(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,$(EXTRA_REPLACE))

CLEANFILES = $(bin_SCRIPTS) $(initd_SCRIPTS)