include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
    $(bin_SCRIPTS:=.in)

bin_SCRIPTS = \
    sipxivr.sh

$(bin_SCRIPTS) : % : %.in
	@$(call SearchAndReplace,$<,$@,ivr_PKGS)

CLEANFILES = $(bin_SCRIPTS)
