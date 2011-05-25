include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(bin_SOURCE:=.in)

OPENACD_VARS = \
	$(filter OPENACD%,$(.VARIABLES))

dist_bin_SCRIPTS = \
	find-erlang-lib

bin_SCRIPTS = \
	sipxopenacd \
	sipxopenacd-setup

setupdir = \
	$(SIPX_LIBEXECDIR)/setup.d

setup_SCRIPTS = \
	sipxopenacd-setup

$(setup_SCRIPTS) $(bin_SCRIPTS) : % : %.in
	@$(call SearchAndReplace, $<, $@, $(OPENACD_VARS))

CLEANFILES = $(setup_SCRIPTS) $(bin_SCRIPTS)
