include $(top_srcdir)/config/utility.am

TESTS = sipx-upgrade-testsuite

bin_SCRIPTS = \
	sipx-config \
	sipx-upgrade

noinst_SCRIPTS = \
	sipx-upgrade-testsuite \
	shell-unit

check_SCRIPTS = sipx-upgrade-testsuite

sipx-upgrade-testsuite : sipx-upgrade shell-unit

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(noinst_SCRIPTS:=.in)

$(noinst_SCRIPTS) $(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(bin_SCRIPTS) $(noinst_SCRIPTS)
