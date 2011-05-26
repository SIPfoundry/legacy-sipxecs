include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
    $(bin_SCRIPTS:=.in)

bin_SCRIPTS = \
    sipxprovision.sh

$(bin_SCRIPTS) : % : %.in
	@$(call SearchAndReplace,$<,$@,provision_PKGS)

CLEANFILES = $(bin_SCRIPTS)
