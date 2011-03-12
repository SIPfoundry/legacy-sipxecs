## Process this file with automake to produce Makefile.in

NTAP_COMMON_CXX_FLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

NTAP_COMMON_LD_FLAGS = \
    -module \
    -version-info ${version_Current}:${version_Revision}:${version_Age}

nobase_lib_LTLIBRARIES = \
    authplugins/libNatTraversalAgent.la

authplugins_libNatTraversalAgent_la_SOURCES = \
    NatTraversalAgent.h \
    NatTraversalAgent.cpp \
    NatMaintainer.h \
    NatMaintainer.cpp \
    NatTraversalRules.h \
    NatTraversalRules.cpp \
    NatTraversalAgentDataTypes.h \
    NatTraversalAgentDataTypes.cpp \
    MediaRelay.h \
    MediaRelay.cpp \
    CallTracker.h \
    CallTracker.cpp \
    SessionContext.h \
    SessionContext.cpp \
    DialogTracker.h \
    DialogTracker.cpp \
    DialogTrackerStates.h \
    DialogTrackerStates.cpp

authplugins_libNatTraversalAgent_la_CXXFLAGS = \
    $(NTAP_COMMON_CXX_FLAGS)

authplugins_libNatTraversalAgent_la_LDFLAGS = \
    $(NTAP_COMMON_LD_FLAGS)

authplugins_libNatTraversalAgent_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@
