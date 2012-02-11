include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(initd_SCRIPTS:=.in) \
	$(bin_SCRIPTS:=.in) \
	$(cfmod_SCRIPTS:=.in)

bin_SCRIPTS = \
	sipxagent \
	sipxagent-log-reader \
	sipxecs-setup

cfmoddir = @SIPX_CFMODULES@
cfmod_SCRIPTS = \
	sipx \
	readfile-if-exists

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
	sipxsupervisor

$(cfmod_SCRIPTS) $(initd_SCRIPTS) $(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(bin_SCRIPTS) $(initd_SCRIPTS)
