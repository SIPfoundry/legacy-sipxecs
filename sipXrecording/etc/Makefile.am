include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(proc_DATA:=.in) \
	$(schema_DATA:=.in)

procdir = $(SIPX_DATADIR)/process.d
proc_DATA = \
	sipxrecording-process.xml

schemadir = $(SIPX_DATADIR)/schema
schema_DATA = \
	conferencebridge.xsd

$(schema_DATA) $(proc_DATA) : % : %.in
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(proc_DATA) $(schema_DATA)
