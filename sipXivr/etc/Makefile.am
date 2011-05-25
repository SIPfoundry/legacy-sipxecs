include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(proc_DATA:=.in) \
	$(schema_DATA:=.in)

procdir = $(SIPX_DATADIR)/process.d
proc_DATA = sipxivr-process.xml

schemadir = @SIPX_DATADIR@/schema
schema_DATA = \
	autoattendants.xsd \
	validusers.xsd 

$(schema_DATA) $(proc_DATA) : % : %.in
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(schema_DATA) $(proc_DATA)
