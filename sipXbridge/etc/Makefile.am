include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(conf_DATA:=.in)

confdir = @SIPX_CONFDIR@/sipxbridge
conf_DATA = \
	sipxbridge.sec.erb

cfinputsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfinputs_DATA = \
	sipxbridge.cf

$(conf_DATA) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

DISTCLEANFILES = $(conf_DATA)
