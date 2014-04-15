include $(top_srcdir)/config/utility.am

cfinputsdir = @SIPX_CFINPUTS@/plugin.d
dist_cfinputs_DATA = \
	sipxrest.cf

EXTRA_DIST = \
	$(conf_DATA:=.in)

confdir = @SIPX_CONFDIR@/sipxrest
conf_DATA = \
	log4j.properties

$(conf_DATA) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)
