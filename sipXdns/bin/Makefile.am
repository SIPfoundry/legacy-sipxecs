include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(libexec_SCRIPTS:=.in)

libexecdir = $(SIPX_LIBEXECDIR)
libexec_SCRIPTS = \
	sipx-dns-zone-maker \
	sipx-dns-validator

$(libexec_SCRIPTS) : % : %.in
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(libexec_SCRIPTS)
