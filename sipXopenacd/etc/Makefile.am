include $(top_srcdir)/config/utility.am

EXTRA_DIST= sipxopenacd.cfdat.in

cfinputsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfinputs_DATA = \
	sipxopenacd.cf

cfdefaultsdir = $(SIPX_CFDATA)/defaults
dist_cfdefaults_DATA = \
	sipxopenacd.cfdat

OPENACD_VARS = \
	$(filter OPENACD%,$(.VARIABLES))

$(dist_cfdefaults_DATA) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@, $(OPENACD_VARS))

DISTCLEANFILES = $(cfdefaults_DATA)
