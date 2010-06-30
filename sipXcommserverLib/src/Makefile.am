## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

SUBDIRS = fastdb sipdb . xsdvalid test sipxrestorebinary sipxpackage sipxswadmin sipxzoneadmin sipxtimemanager sipxchangetimezone sipxcdrpostgres

INCLUDES = -I$(top_srcdir)/include

SipXscripts = \
	sipx-utils.sh

EXTRA_DIST = \
	$(foreach sipxScript,$(SipXscripts),$(sipxScript).in)

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
	fastdb/libfastdb.la \
	sipdb/libsipdb.la

libsipXcommserver_la_SOURCES = \
    $(top_builddir)/config/sipxcommserverlib-buildstamp.h \
    $(top_builddir)/config/sipxcommserverlib-buildstamp.cpp \
    alarm/Alarm.cpp \
    alarm/AlarmRequestTask.cpp \
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
	 sipXecsService/SipXecsService.cpp \
    sipXecsService/SipNonceDb.cpp \
	 sipXecsService/SharedSecret.cpp

all: $(foreach sipxScript,$(SipXscripts),$(sipxScript))

install-exec-hook: \
	$(foreach sipxScript,$(SipXscripts),$(DESTDIR)$(libexecdir)/sipXecs/$(sipxScript))

$(foreach sipxScript,$(SipXscripts),$(DESTDIR)$(libexecdir)/sipXecs/$(sipxScript)) : \
		$(DESTDIR)$(libexecdir)/sipXecs/% : %
	$(INSTALL) -D -m 755 $< $@

$(foreach sipxScript,$(SipXscripts),$(sipxScript)) : % : $(srcdir)/%.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
	chmod +x $@
