include $(top_srcdir)/config/utility.am

# EXTRA_DIST is part of automake, this gets files into tarball which inturn is
# important for rpm builds. Sometimes you can use dist_ suffix on variables to
# avoid using EXTRA_DIST variable.
EXTRA_DIST = \
	$(bin_SCRIPTS:=.in)

bin_SCRIPTS = \
	sipxexample-script

CUSTOM_VAR = Hello from GNU Make
$(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,CUSTOM_VAR)

CLEANFILES = $(initd_SCRIPTS)

