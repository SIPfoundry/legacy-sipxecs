include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(setup_SCRIPTS:=.in)

bin_SCRIPTS = \
	sipxconfig.sh \
	sipx-mrtg-init \
	sipx-snmp-init \
	sipXmrtg.sh \
	sipx-time-manager \
	sipx-tunnel

setupdir = @SIPX_LIBEXECDIR@/setup.d
setup_SCRIPTS = \
	sipxconfig-setup

EXTRA_REPLACE = \
	MRTG_BINARY \
	DATE_BINARY \
	web_PKGS

$(setup_SCRIPTS) $(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,$(EXTRA_REPLACE))

CLEANFILES = $(bin_SCRIPTS) $(setup_SCRIPTS)