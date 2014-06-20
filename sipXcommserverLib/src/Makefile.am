SUBDIRS = \
	sipdb \
	. \
	xsdvalid \
	sipxpackage \
	sipxswadmin \
	sipxcdrpostgres \
	test

INCLUDES = -I$(top_srcdir)/include

lib_LTLIBRARIES = libsipXcommserver.la

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=2
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXcommserver_la_LDFLAGS = \
	-version-info ${version_Current}:${version_Revision}:${version_Age} \
	@ODBC_LDFLAGS@

# @TODO - ideally, these would be passed only to the SipXecsService build,
#         which could be done by reorganizing this so that each subdirectory
#         built a convenience library and this level just linked them together.
libsipXcommserver_la_CXXFLAGS = \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_VARDIR=\"@SIPX_VARDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\" \
	-DSIPX_DATADIR=\"@SIPX_DATADIR@\" \
	-DSIPX_BINDIR=\"@SIPX_BINDIR@\" \
	-DSIPX_LIBEXECDIR=\"@SIPX_LIBEXECDIR@\" \
	-DSIPXPBXUSER=\"@SIPXPBXUSER@\" \
	-DSIPXPBXGROUP=\"@SIPXPBXGROUP@\" \
	-DSIPXECS_NAME=\"@SIPXECS_NAME@\" \
	@ODBC_CXXFLAGS@

libsipXcommserver_la_LIBADD = \
	@SIPXTACK_LIBS@ \
	@ODBC_LIBS@ \
	sipdb/libsipdb.la \
	-lboost_regex-mt \
	-lboost_program_options-mt \
	-lPocoFoundation -lPocoUtil -lPocoNet -lPocoNetSSL -lPocoXML \
	-lleveldb

libsipXcommserver_la_SOURCES = \
    digitmaps/UrlMapping.cpp \
    digitmaps/FallbackRulesUrlMapping.cpp \
    digitmaps/AuthRulesUrlMapping.cpp \
    digitmaps/EmergencyRulesUrlMapping.cpp \
    digitmaps/MappingRulesUrlMapping.cpp \
    digitmaps/Patterns.cpp \
    filereader/OrbitFileReader.cpp \
    filereader/RefreshingFileReader.cpp \
    configrpc/ConfigRPC.cpp \
    odbc/OdbcWrapper.cpp \
    persist/SipPersistentSubscriptionMgr.cpp \
    sipXecsService/daemon.cpp \
    sipXecsService/SipXecsService.cpp \
    sipXecsService/SipNonceDb.cpp \
    sipXecsService/SharedSecret.cpp \
    sipXecsService/SipXApplication.cpp
