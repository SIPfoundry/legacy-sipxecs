include $(top_srcdir)/config/utility.am

SUBDIRS = \
	alarms \
	mibs \
	.

EXTRA_DIST = \
	$(schema_DATA:=.in)

schemadir = @SIPX_DATADIR@/schema
schema_DATA = \
	sipXecs-process.xsd \
	alarm.xsd \
	alarm-group.xsd

$(schema_DATA) : % : %.in
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = \
	$(schema_DATA)

