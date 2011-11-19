include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(cfdefaults_DATA:=.in)

confdir = $(SIPX_CONFDIR)
dist_conf_DATA = \
	mongo-client.ini

cfdefaultsdir = $(SIPX_CFDATA)/defaults
cfdefaults_DATA = \
	mongodb.cfdat

cfpluginsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfplugins_DATA = \
	mongodb.cf

$(cfdefaults_DATA) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(cfdefaults_DATA)
