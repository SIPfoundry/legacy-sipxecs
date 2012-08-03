include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(conf_DATA:=.in)

confdir = @SIPX_LIBEXECDIR@
conf_DATA = \
	openfire-config

$(conf_DATA) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,plugin_PKGS provider_PKGS_RT)

CLEANFILES = $(conf_DATA)
