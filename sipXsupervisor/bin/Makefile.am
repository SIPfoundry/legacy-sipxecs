include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(initd_SCRIPTS:=.in) \
	$(bin_SCRIPTS:=.in) \
	$(cf_SCRIPTS:=.in)

bin_SCRIPTS = \
	sipxsupervisor.sh \
	sipxagent

cfmoddir = @SIPX_CFMODULES@
cfmod_SCRIPTS = sipx

initddir = @SERVICEDIR@
initd_SCRIPTS = \
	sipxsupervisord

$(cfmod_SCRIPTS) $(initd_SCRIPTS) $(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)


CLEANFILES = $(bin_SCRIPTS) $(initd_SCRIPTS)
