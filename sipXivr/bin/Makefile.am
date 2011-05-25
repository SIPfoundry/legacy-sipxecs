include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
    $(bin_SCRIPTS:=.in)

bin_SCRIPTS = \
    sipxivr.sh

$(bin_SCRIPTS) : % : %.in
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(bin_SCRIPTS)
