include $(top_srcdir)/config/utility.am

bin_SCRIPTS = \
	sipxcallresolver-agent.sh \
	sipxcallresolver.sh

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(setup_SCRIPTS:=.in)

setupdir = @SIPX_LIBEXECDIR@/setup.d
setup_SCRIPTS = \
	sipxcallresolver-setup

$(setup_SCRIPTS) $(bin_SCRIPTS) : % : %.in
	@$(call SearchAndReplace,$<,$@,STUNNEL)

CLEANFILES = $(bin_SCRIPTS) $(setup_SCRIPTS)
