include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(conf_DATA:=.in)

cfinputsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfinputs_DATA = \
	sipxproxy.cf

confdir = $(SIPX_CONFDIR)/sipxproxy
conf_DATA = \
	sipxproxy.sec.erb

$(conf_DATA) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(conf_DATA)

