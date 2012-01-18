include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(schema_DATA:=.in)

cfinputsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfinputs_DATA = \
	sipxivr.cf

schemadir = @SIPX_DATADIR@/schema
schema_DATA = \
	autoattendants.xsd \
	validusers.xsd 

$(schema_DATA) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(schema_DATA)
