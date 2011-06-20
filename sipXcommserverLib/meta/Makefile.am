include $(top_srcdir)/config/utility.am

schema_DATA = \
	credential.xsd \
	registration.xsd \
	subscription.xsd

dist_schema_DATA = \
	authrules.xsd \
	urlmap.xsd

EXTRA_DIST = \
	$(schema_DATA:=.in)

schemadir = @SIPX_DATADIR@/schema
$(schema_DATA) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(schema_DATA)
