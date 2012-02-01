include $(top_srcdir)/config/utility.am

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
    sipxopenacd

EXTRA_DIST = \
	$(initd_SCRIPTS:=.in)

OPENACD_VARS = \
	$(filter OPENACD%,$(.VARIABLES))

$(initd_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@, $(OPENACD_VARS))

CLEANFILES = $(initd_SCRIPTS)
