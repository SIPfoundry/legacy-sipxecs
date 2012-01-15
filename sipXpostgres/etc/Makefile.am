include $(top_srcdir)/config/utility.am

cfdefaultsdir = $(SIPX_CFDATA)/defaults
cfdefaults_DATA = \
	postgres.cfdat

postgresvars = \
	$(filter PG%,$(.VARIABLES)) \
	$(filter POSTGRES%,$(.VARIABLES))

postgres.cfdat : Makefile
	! test -f $@ || rm $@
	$(foreach V,$(postgresvars), echo '=$(V)=$($(V))' >> $@;)

cfinputsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfinputs_DATA = \
	postgres.cf

CLEANFILES = postgres.cfdat
