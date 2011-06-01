include $(top_srcdir)/config/utility.am

INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/config

bin_PROGRAMS = \
	xsdvalid

xsdvalid_LDADD = \
	@SIPXPORT_LIBS@ \
	@XERCES_LIBS@

xsdvalid_CPPFLAGS = \
	@XERCES_CXXFLAGS@ \
	$(AM_CFLAGS)

xsdvalid_SOURCES = \
	xsdvalid.cpp

schema_DATA = \
	xml.xsd

EXTRA_DIST = \
	$(schema_DATA:=.in)

schemadir = @SIPX_DATADIR@/schema

$(schema_DATA) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(schema_DATA)
