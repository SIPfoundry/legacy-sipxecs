include $(top_srcdir)/config/utility.am


bin_SCRIPTS = \
	sipx-config \
	sipx-upgrade

noinst_SCRIPTS = \
	shell-unit

sipx-upgrade-testsuite : sipx-upgrade shell-unit

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(noinst_SCRIPTS:=.in)

$(noinst_SCRIPTS) $(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(bin_SCRIPTS) $(noinst_SCRIPTS)
