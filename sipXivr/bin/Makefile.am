include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(initd_SCRIPTS:=.in) \
	$(bin_SCRIPTS:=.in)

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
	sipxivr

bindir = @SIPX_BINDIR@
bin_SCRIPTS = \
  sipxivr-archive \
  voicemail_clean

$(bin_SCRIPTS) $(initd_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,ivr_PKGS)

DISTCLEANFILES = $(initd_SCRIPTS) $(bin_SCRIPTS)
