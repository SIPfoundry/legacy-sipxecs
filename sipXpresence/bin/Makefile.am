include $(top_srcdir)/config/utility.am

bin_SCRIPTS = \
    sipxpresence.sh

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in)

$(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(bin_SCRIPTS)
