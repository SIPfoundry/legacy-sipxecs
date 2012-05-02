include $(top_srcdir)/config/utility.am

# script doesn't work, but keep in case it can be
# restored someday -- Douglas
EXTRA_DIST = \
	sipx-snapshot-rls.in

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
    sipxrls

EXTRA_DIST = \
	$(initd_SCRIPTS:=.in)

$(initd_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(initd_SCRIPTS)
