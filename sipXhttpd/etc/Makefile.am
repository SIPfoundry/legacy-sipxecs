include $(top_srcdir)/config/utility.am

cfdefaultsdir = $(SIPX_CFDATA)/defaults
cfdefaults_DATA = \
	apache.cfdat

cfconfdir = $(SIPX_CONFDIR)/apache
dist_cfconf_DATA = ssl.conf

apachevars = \
	$(filter APACHE%,$(.VARIABLES))

apache.cfdat : Makefile
	! test -f $@ || rm $@
	$(foreach V,$(apachevars), echo '=$(V)=$($(V))' >> $@;)

cfinputsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfinputs_DATA = \
	zz_apache.cf

CLEANFILES = apache.cfdat
