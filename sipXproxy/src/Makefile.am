## Process this file with automake to produce Makefile.in

SUBDIRS = . test

## Process this file with automake to produce Makefile.in

SIPXPROXY_COMMON_CXX_FLAGS = \
	-I$(top_srcdir)/include \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DPOSTGRESQL_USER=\"@POSTGRESQL_USER@\" \
	-DSIPX_LIBDIR=\"@SIPX_LIBDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\"

bin_PROGRAMS = sipXproxy wsrouter

sipXproxy_CXXFLAGS = \
	$(SIPXPROXY_COMMON_CXX_FLAGS)

sipXproxy_LDFLAGS = \
    -shared \
    -rdynamic

sipXproxy_LDADD = \
	libsipXproxy.la \
	@SIPXCOMMSERVER_LIBS@ \
        -lzmq \
        -lmongoclient \
        -lboost_system-mt

sipXproxy_SOURCES =  \
	sipXproxymain.cpp


wsrouter_CXXFLAGS = \
	$(SIPXPROXY_COMMON_CXX_FLAGS)

wsrouter_LDFLAGS = \
    -shared \
    -rdynamic

wsrouter_LDADD = \
	libsipXproxy.la \
	@SIPXCOMMSERVER_LIBS@ \
        -lzmq \
        -lmongoclient \
        -lboost_system-mt

wsrouter_SOURCES =  \
  wsrouter/WSRouterMain.cpp \
  wsrouter/WSRouter.h \
  wsrouter/WSRouter.cpp

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

lib_LTLIBRARIES = libsipXproxy.la

libsipXproxy_la_LDFLAGS = \
    -shared \
    -version-info ${version_Current}:${version_Revision}:${version_Age} \
    -ldb_cxx \
    -lcares \
    -lrutil \
    -lresip \
    -ldum \
    -lrepro


libsipXproxy_la_SOURCES = \
	SipBridgeRouter.cpp \
	SipBridgeRouter.h \
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
   CallStateEventWriter_DB.cpp \
   UnifiedProxyPlugin.h \
   UnifiedProxyPlugin.cpp \
   ReproGlue.cpp

libsipXproxy_la_CXXFLAGS = \
	$(SIPXPROXY_COMMON_CXX_FLAGS)
	

        