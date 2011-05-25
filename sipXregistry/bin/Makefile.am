include $(top_srcdir)/config/utility.am

bin_SCRIPTS = \
    sipregistrar.sh

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in)

$(bin_SCRIPTS) : % : %.in
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(bin_SCRIPTS)
