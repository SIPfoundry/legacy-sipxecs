include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
    $(bin_SCRIPTS:=.in)

bin_SCRIPTS = \
    sipxacccode.sh

$(bin_SCRIPTS) : % : %.in
	@$(call SearchAndReplace,$<,$@,sipxacc_PKGS)

CLEANFILES = $(bin_SCRIPTS)
