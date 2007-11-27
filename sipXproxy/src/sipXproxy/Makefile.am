## Process this file with automake to produce Makefile.in

SIPXPROXY_COMMON_CXX_FLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/src/cse \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

bin_PROGRAMS = sipXproxy

sipXproxy_CXXFLAGS = \
	$(SIPXPROXY_COMMON_CXX_FLAGS)

sipXproxy_LDFLAGS = \
    -rdynamic

sipXproxy_LDADD = \
	libsipXproxy.la \
	../cse/libcse.la \
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
## so that they can be more easily used in the unit tests in ../test
## The convenience library is static, and is never installed - it is statically 
## linked to create the executable above.
noinst_LTLIBRARIES = libsipXproxy.la

libsipXproxy_la_SOURCES = \
	SipRouter.cpp \
	ForwardRules.cpp \
	SipRouter.h \
	ForwardRules.h \
	AuthPlugin.h \
	AuthPlugin.cpp \
	RouteState.h \
	RouteState.cpp \
	SipXProxyCseObserver.h \
	SipXProxyCseObserver.cpp

libsipXproxy_la_CXXFLAGS = \
	$(SIPXPROXY_COMMON_CXX_FLAGS)

AUTHPLUGIN_COMMON_LD_FLAGS = \
    -module \
    -version-info ${version_Current}:${version_Revision}:${version_Age}
    
nobase_lib_LTLIBRARIES = \
    authplugins/libCallerAlias.la \
    authplugins/libTransferControl.la \
    authplugins/libEnforceAuthRules.la

authplugins_libCallerAlias_la_SOURCES = \
    CallerAlias.h \
    CallerAlias.cpp

authplugins_libCallerAlias_la_CXXFLAGS = \
    $(SIPXPROXY_COMMON_CXX_FLAGS)

authplugins_libCallerAlias_la_LDFLAGS = \
    $(AUTHPLUGIN_COMMON_LD_FLAGS)

authplugins_libCallerAlias_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@

authplugins_libEnforceAuthRules_la_SOURCES = \
    EnforceAuthRules.h \
    EnforceAuthRules.cpp

authplugins_libEnforceAuthRules_la_CXXFLAGS = \
    $(SIPXPROXY_COMMON_CXX_FLAGS)

authplugins_libEnforceAuthRules_la_LDFLAGS = \
    $(AUTHPLUGIN_COMMON_LD_FLAGS)

authplugins_libEnforceAuthRules_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@

authplugins_libTransferControl_la_SOURCES = \
    TransferControl.h \
    TransferControl.cpp

authplugins_libTransferControl_la_CXXFLAGS = \
    $(AUTHPROXY_COMMON_CXX_FLAGS)

authplugins_libTransferControl_la_LDFLAGS = \
    $(AUTHPLUGIN_COMMON_LD_FLAGS)

authplugins_libTransferControl_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@
