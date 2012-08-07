include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(conf_DATA:=.in) \
	$(bin_SCRIPTS:=.in)

confdir = @SIPX_LIBEXECDIR@
conf_DATA = \
	openfire-config
	
bindir = @SIPX_BINDIR@
bin_SCRIPTS= \
	openfire-setprop \
	openfire-config-prop

$(conf_DATA) $(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,plugin_PKGS provider_PKGS_RT)

CLEANFILES = $(conf_DATA) $(bin_SCRIPTS)
