include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(initd_SCRIPTS:=.in)

bin_SCRIPTS = \
	mongodb-repair

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
	mongod-arbiter

$(initd_SCRIPTS) $(bin_SCRIPTS) : % : %.in
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(initd_SCRIPTS) $(bin_SCRIPTS)
