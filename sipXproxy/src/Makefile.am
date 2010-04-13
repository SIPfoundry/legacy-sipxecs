## Process this file with automake to produce Makefile.in

SUBDIRS = . cdr test

## Process this file with automake to produce Makefile.in

SIPXPROXY_COMMON_CXX_FLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DPOSTGRESQL_USER=\"@POSTGRESQL_USER@\"

bin_PROGRAMS = sipXproxy

sipXproxy_CXXFLAGS = \
	$(SIPXPROXY_COMMON_CXX_FLAGS)

sipXproxy_LDFLAGS = \
    -rdynamic

sipXproxy_LDADD = \
	libsipXproxy.la \
	@SIPXCOMMSERVER_LIBS@

sipXproxy_SOURCES =  \
	$(top_builddir)/config/sipxproxy-buildstamp.h \
	$(top_builddir)/config/sipxproxy-buildstamp.cpp \
	sipXproxymain.cpp

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

## The objects in the sipXproxy are built as a libtool convenience library
## so that they can be more easily used in the unit tests in the ./test and
## ../lib/authplugins/test directories.
## The convenience library is static, and is never installed - it is statically
## linked to create the executable above.
noinst_LTLIBRARIES = libsipXproxy.la

libsipXproxy_la_LDFLAGS = \
    -version-info ${version_Current}:${version_Revision}:${version_Age}

libsipXproxy_la_SOURCES = \
	SipRouter.cpp \
	ForwardRules.cpp \
	ForwardRules.h \
	AuthPlugin.cpp \
	RouteState.cpp \
	SipXProxyCseObserver.h \
	SipXProxyCseObserver.cpp \
   CallStateEventBuilder.h \
   CallStateEventBuilder.cpp \
   CallStateEventBuilder_XML.h \
   CallStateEventBuilder_XML.cpp \
   CallStateEventBuilder_DB.h \
   CallStateEventBuilder_DB.cpp \
   CallStateEventWriter.h \
   CallStateEventWriter.cpp \
   CallStateEventWriter_XML.h \
   CallStateEventWriter_XML.cpp \
   CallStateEventWriter_DB.h \
   CallStateEventWriter_DB.cpp

libsipXproxy_la_CXXFLAGS = \
	$(SIPXPROXY_COMMON_CXX_FLAGS)
