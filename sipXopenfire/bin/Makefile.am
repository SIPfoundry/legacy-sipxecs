include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(setup_SCRIPTS:=.in)

bin_SCRIPTS = \
	sipxopenfire.sh

setupdir = $(SIPX_LIBEXECDIR)/setup.d
setup_SCRIPTS = \
	sipxopenfire-setup.sh

$(bin_SCRIPTS) $(setup_SCRIPTS) : % : %.in
	@$(call SearchAndReplace, $<, $@, OPENFIRE_HOME)

CLEANFILES = $(bin_SCRIPTS) $(setup_SCRIPTS)
