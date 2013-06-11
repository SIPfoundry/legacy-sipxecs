include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(libexec_SCRIPTS:=.in)

bin_SCRIPTS = \
	sipxtrap

libexecdir = @SIPX_LIBEXECDIR@
libexec_SCRIPTS = \
    snmptrap-email-handler \
    snmp-fix-process \
    snmp-fix-process-cron

$(libexec_SCRIPTS) $(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(bin_SCRIPTS) $(libexec_SCRIPTS)
