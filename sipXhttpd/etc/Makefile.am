include $(top_srcdir)/config/utility.am

cfdefaultsdir = $(SIPX_CFDATA)/defaults
cfdefaults_DATA = \
	apache.cfdat

apachevars = \
	$(filter APACHE%,$(.VARIABLES))

apache.cfdat : Makefile
	! test -f $@ || rm $@
	$(foreach V,$(apachevars), echo '=$(V)=$($(V))' >> $@;)

cfinputsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfinputs_DATA = \
	zz_apache.cf

CLEANFILES = apache.cfdat
