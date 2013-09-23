## Process this file with automake to produce Makefile.in

SUBDIRS = . NatTraversalAgent test

AUTHPLUGIN_COMMON_CXX_FLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include \
	-DSIPX_LIBDIR=\"@SIPX_LIBDIR@\"

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if -lmongoclient \
    -lboost_system-mtany implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

AUTHPLUGIN_COMMON_LD_FLAGS = \
    -module \
    -version-info ${version_Current}:${version_Revision}:${version_Age}

nobase_lib_LTLIBRARIES = \
    authplugins/libCallerAlias.la \
    authplugins/libCallerAlertInfo.la \
    authplugins/libTransferControl.la \
    authplugins/libEnforceAuthRules.la \
    authplugins/libEmergencyLineIdentifier.la \
    authplugins/libRequestLinter.la \
    authplugins/libMSFT_ExchangeTransferHack.la \
    authplugins/libSubscriptionAuth.la \
    authplugins/libCallDestination.la

authplugins_libCallerAlias_la_SOURCES = \
    CallerAlias.h \
    CallerAlias.cpp

authplugins_libCallerAlias_la_CXXFLAGS = \
    $(AUTHPLUGIN_COMMON_CXX_FLAGS)

authplugins_libCallerAlias_la_LDFLAGS = \
    $(AUTHPLUGIN_COMMON_LD_FLAGS)

authplugins_libCallerAlias_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@ \
    -lmongoclient \
    -lboost_system-mt


authplugins_libCallerAlertInfo_la_SOURCES = \
    CallerAlertInfo.h \
    CallerAlertInfo.cpp

authplugins_libCallerAlertInfo_la_CXXFLAGS = \
    $(AUTHPLUGIN_COMMON_CXX_FLAGS)

authplugins_libCallerAlertInfo_la_LDFLAGS = \
    $(AUTHPLUGIN_COMMON_LD_FLAGS)

authplugins_libCallerAlertInfo_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@


authplugins_libEnforceAuthRules_la_SOURCES = \
    EnforceAuthRules.h \
    EnforceAuthRules.cpp

authplugins_libEnforceAuthRules_la_CXXFLAGS = \
    $(AUTHPLUGIN_COMMON_CXX_FLAGS)

authplugins_libEnforceAuthRules_la_LDFLAGS = \
    $(AUTHPLUGIN_COMMON_LD_FLAGS)

authplugins_libEnforceAuthRules_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@ \
    -lmongoclient \
    -lboost_system-mt

authplugins_libRequestLinter_la_SOURCES = \
    RequestLinter.h \
    RequestLinter.cpp

authplugins_libRequestLinter_la_CXXFLAGS = \
    $(AUTHPLUGIN_COMMON_CXX_FLAGS)

authplugins_libRequestLinter_la_LDFLAGS = \
    $(AUTHPLUGIN_COMMON_LD_FLAGS)

authplugins_libRequestLinter_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@ \
    -lmongoclient \
    -lboost_system-mt

authplugins_libTransferControl_la_SOURCES = \
    TransferControl.h \
    TransferControl.cpp

authplugins_libTransferControl_la_CXXFLAGS = \
    $(AUTHPLUGIN_COMMON_CXX_FLAGS)

authplugins_libTransferControl_la_LDFLAGS = \
    $(AUTHPLUGIN_COMMON_LD_FLAGS)

authplugins_libTransferControl_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@ \
    -lmongoclient \
    -lboost_system-mt

authplugins_libMSFT_ExchangeTransferHack_la_SOURCES = \
    MSFT_ExchangeTransferHack.h \
    MSFT_ExchangeTransferHack.cpp

authplugins_libMSFT_ExchangeTransferHack_la_CXXFLAGS = \
    $(AUTHPLUGIN_COMMON_CXX_FLAGS)

authplugins_libMSFT_ExchangeTransferHack_la_LDFLAGS = \
    $(AUTHPLUGIN_COMMON_LD_FLAGS)

authplugins_libMSFT_ExchangeTransferHack_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@ \
    -lmongoclient \
    -lboost_system-mt

authplugins_libSubscriptionAuth_la_SOURCES = \
    SubscriptionAuth.h \
    SubscriptionAuth.cpp

authplugins_libSubscriptionAuth_la_CXXFLAGS = \
    $(AUTHPLUGIN_COMMON_CXX_FLAGS)

authplugins_libSubscriptionAuth_la_LDFLAGS = \
    $(AUTHPLUGIN_COMMON_LD_FLAGS)

authplugins_libSubscriptionAuth_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@ \
    -lmongoclient \
    -lboost_system-mt

authplugins_libCallDestination_la_SOURCES = \
    CallDestination.h \
    CallDestination.cpp

authplugins_libCallDestination_la_CXXFLAGS = \
    $(AUTHPLUGIN_COMMON_CXX_FLAGS)

authplugins_libCallDestination_la_LDFLAGS = \
    $(AUTHPLUGIN_COMMON_LD_FLAGS)

authplugins_libCallDestination_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@ \
    -lmongoclient \
    -lboost_system-mt

authplugins_libEmergencyLineIdentifier_la_SOURCES = \
    EmergencyLineIdentifier.h \
    EmergencyLineIdentifier.cpp

authplugins_libEmergencyLineIdentifier_la_CXXFLAGS = \
    $(AUTHPLUGIN_COMMON_CXX_FLAGS)

authplugins_libEmergencyLineIdentifier_la_LDFLAGS = \
    $(AUTHPLUGIN_COMMON_LD_FLAGS)

authplugins_libEmergencyLineIdentifier_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@ \
    -lmongoclient \
    -lboost_system-mt