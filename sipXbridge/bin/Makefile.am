include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

bin_SCRIPTS = \
	sipxbridge.sh

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in)

$(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,bridge_PKGS)

CLEANFILES = $(bin_SCRIPTS)
