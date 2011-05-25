include $(top_srcdir)/config/utility.am

bin_SCRIPTS = \
    sipxrls.sh \
    sipx-snapshot-rls

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in)

$(bin_SCRIPTS) : % : %.in
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(bin_SCRIPTS)
