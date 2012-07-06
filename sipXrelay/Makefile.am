include config/utility.am
include config/project.am
include common.am

SUBDIRS = \
	src \
	.

@PACKAGE@_EXTRA_DIST = \
	CONTRIBUTORS \
	$(initd_SCRIPTS:=.in) \
	$(conf_DATA:=.in)
	
confdir = @SIPX_CONFDIR@/sipxrelay
conf_DATA = \
	sipxrelay.sec.erb	

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
	sipxrelay

$(conf_DATA) $(initd_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,relay_PKGS)

cfinputsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfinputs_DATA = \
	sipxrelay.cf

DISTCLEANFILES = $(initd_SCRIPTS) $(conf_DATA)
