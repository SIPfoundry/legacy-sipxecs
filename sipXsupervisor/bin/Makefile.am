include $(top_srcdir)/config/utility.am

bin_SCRIPTS = \
	sipxsupervisor.sh

$(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

EXTRA_DIST = $(bin_SCRIPTS:=.in)

CLEANFILES = $(bin_SCRIPTS)
