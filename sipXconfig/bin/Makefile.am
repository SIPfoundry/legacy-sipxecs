include $(top_srcdir)/config/utility.am

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

CONF_VARS = \
	MRTG_BINARY \
	DATE_BINARY \
	SERVICEDIR \
	POSTGRESQL_USER \
	CLASSPATH_RUN

$(setup_SCRIPTS) $(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,$(CONF_VARS))

CLEANFILES = $(bin_SCRIPTS) $(setup_SCRIPTS)