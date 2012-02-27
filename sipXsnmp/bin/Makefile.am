include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(libexec_SCRIPTS:=.in)

bin_SCRIPTS = \
	sipxtrap

libexecdir = @SIPX_LIBEXECDIR@
libexec_SCRIPTS = \
	sipxtrap-handler

$(libexec_SCRIPTS) $(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(bin_SCRIPTS) $(libexec_SCRIPTS)
