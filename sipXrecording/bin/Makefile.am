include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
    $(bin_SCRIPTS:=.in)

bin_SCRIPTS = \
    sipxrecording.sh

$(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(bin_SCRIPTS)
