include $(top_srcdir)/config/utility.am

bin_SCRIPTS = \
    sipstatus.sh

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in)

$(bin_SCRIPTS) : % : %.in
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(bin_SCRIPTS)
