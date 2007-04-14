## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sipauthproxy

AUTHPROXY_COMMON_CXX_FLAGS = \
	-I$(top_builddir)/config \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipauthproxy_CXXFLAGS = \
	$(AUTHPROXY_COMMON_CXX_FLAGS) \
	-I$(top_srcdir)/src/cse

sipauthproxy_LDFLAGS = \
	-rdynamic

sipauthproxy_LDADD = \
	libauthproxy.la \
	$(top_builddir)/src/cse/libcse.la \
	@SIPXCOMMSERVER_LIBS@

sipauthproxy_SOURCES =  \
   $(top_builddir)/config/sipxproxy-buildstamp.h \
   $(top_builddir)/config/sipxproxy-buildstamp.cpp \
   sipauthproxymain.cpp

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

## The objects in the authproxy are built as a libtool convenience library
## so that they can be more easily used in the unit tests in ../test
## The convenience library is static, and is never installed - it is statically 
## linked to create the executable above.
noinst_LTLIBRARIES = libauthproxy.la

libauthproxy_la_SOURCES = \
   SipAaa.cpp \
	SipAaa.h \
	AuthPlugin.h \
	RouteState.h \
	RouteState.cpp \
	AuthProxyCseObserver.h \
	AuthProxyCseObserver.cpp

libauthproxy_la_CXXFLAGS = \
	$(AUTHPROXY_COMMON_CXX_FLAGS) \
	-I$(top_srcdir)/src/cse

AUTHPLUGIN_COMMON_LD_FLAGS = \
	-module \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

nobase_lib_LTLIBRARIES = \
	sipxauthproxy/libCallerAlias.la \
	sipxauthproxy/libEnforceAuthRules.la

sipxauthproxy_libCallerAlias_la_SOURCES = \
	CallerAlias.h \
	CallerAlias.cpp

sipxauthproxy_libCallerAlias_la_CXXFLAGS = \
	$(AUTHPROXY_COMMON_CXX_FLAGS)

sipxauthproxy_libCallerAlias_la_LDFLAGS = \
	$(AUTHPLUGIN_COMMON_LD_FLAGS)

sipxauthproxy_libCallerAlias_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

sipxauthproxy_libEnforceAuthRules_la_SOURCES = \
	EnforceAuthRules.h \
	EnforceAuthRules.cpp

sipxauthproxy_libEnforceAuthRules_la_CXXFLAGS = \
	$(AUTHPROXY_COMMON_CXX_FLAGS)

sipxauthproxy_libEnforceAuthRules_la_LDFLAGS = \
	$(AUTHPLUGIN_COMMON_LD_FLAGS)

sipxauthproxy_libEnforceAuthRules_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@




