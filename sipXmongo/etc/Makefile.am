include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(conf_DATA:=.in)

cfpluginsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfplugins_DATA = \
	mongodb.cf

confdir = $(SIPX_CONFDIR)/mongo

conf_DATA = \
	mongo.sec.erb

$(conf_DATA) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(conf_DATA)
