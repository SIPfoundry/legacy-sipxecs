
SUBDIRS = test

lib_LTLIBRARIES = \
	libRedirectorAliasDB.la \
	libRedirectorAuthRouter.la \
	libRedirectorMapping.la \
    	libRedirectorFallback.la \
	libRedirectorPickUp.la \
	libRedirectorRegDB.la \
	libRedirectorSubscribe.la \
	libRedirectorUserParam.la \
	libRedirectorENUM.la \
	libRedirectorISN.la \
	libRedirectorJoin.la \
    libRedirectorTest.la \
    libRedirectorPresenceRouting.la \
    libRedirectorTimeOfDay.la


INCLUDES = -I$(top_srcdir)/include

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

# Note that adding new @*_LIBS@ symbols to the *_LIBADD lists may require
# adding SFAC_LIB_* macros (defined in ../../../config/sipXlib.m4)
# to ../../configure.ac to establish the substitution for the @*_LIBS@ symbols.

# SipRedirectorAliasDB

libRedirectorAliasDB_la_LDFLAGS = \
	-module \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libRedirectorAliasDB_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libRedirectorAliasDB_la_CXXFLAGS = \
	-I$(top_builddir)/config

libRedirectorAliasDB_la_SOURCES =  \
	SipRedirectorAliasDB.cpp \
	SipRedirectorAliasDB.h

# SipRedirectorAuthRouter

libRedirectorAuthRouter_la_LDFLAGS = \
	-module \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libRedirectorAuthRouter_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libRedirectorAuthRouter_la_CXXFLAGS = \
	-I$(top_builddir)/config

libRedirectorAuthRouter_la_SOURCES =  \
	SipRedirectorAuthRouter.cpp \
	SipRedirectorAuthRouter.h

# SipRedirectorMapping

libRedirectorMapping_la_LDFLAGS = \
	-module \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libRedirectorMapping_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libRedirectorMapping_la_CXXFLAGS = \
	-I$(top_builddir)/config

libRedirectorMapping_la_SOURCES =  \
	SipRedirectorMapping.cpp \
	SipRedirectorMapping.h

# SipRedirectorFallback

libRedirectorFallback_la_LDFLAGS = \
    -module \
    -version-info ${version_Current}:${version_Revision}:${version_Age}

libRedirectorFallback_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@

libRedirectorFallback_la_CXXFLAGS = \
    -I$(top_builddir)/config

libRedirectorFallback_la_SOURCES =  \
    SipRedirectorFallback.cpp \
    SipRedirectorFallback.h

# SipRedirectorPickUp

libRedirectorPickUp_la_LDFLAGS = \
	-module \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libRedirectorPickUp_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libRedirectorPickUp_la_CXXFLAGS = \
	-I$(top_builddir)/config

libRedirectorPickUp_la_SOURCES =  \
	SipRedirectorPickUp.cpp \
	SipRedirectorPickUp.h

# SipRedirectorSubscribe

libRedirectorSubscribe_la_LDFLAGS = \
	-module \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libRedirectorSubscribe_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libRedirectorSubscribe_la_CXXFLAGS = \
	-I$(top_builddir)/config

libRedirectorSubscribe_la_SOURCES =  \
	SipRedirectorSubscribe.cpp \
	SipRedirectorSubscribe.h

# SipRedirectorRegDB

libRedirectorRegDB_la_LDFLAGS = \
	-module \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libRedirectorRegDB_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libRedirectorRegDB_la_CXXFLAGS = \
	-I$(top_builddir)/config

libRedirectorRegDB_la_SOURCES =  \
	SipRedirectorRegDB.cpp \
	SipRedirectorRegDB.h

# Experimental redirectors

# SipRedirectorENUM
# Experimental redirector to implement ENUM dialing.

libRedirectorENUM_la_LDFLAGS = \
	-module \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libRedirectorENUM_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libRedirectorENUM_la_CXXFLAGS = \
	-I$(top_builddir)/config \
	-DRES_PARSE_NAPTR

libRedirectorENUM_la_SOURCES =  \
	SipRedirectorENUM.cpp \
	SipRedirectorENUM.h

# SipRedirectorISN
# Experimental redirector to implement ISN dialing.

libRedirectorISN_la_LDFLAGS = \
	-module \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libRedirectorISN_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libRedirectorISN_la_CXXFLAGS = \
	-I$(top_builddir)/config \
	-DRES_PARSE_NAPTR

libRedirectorISN_la_SOURCES =  \
	SipRedirectorISN.cpp \
	SipRedirectorISN.h

# SipRedirectorJoin

libRedirectorJoin_la_LDFLAGS = \
	-module \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libRedirectorJoin_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libRedirectorJoin_la_CXXFLAGS = \
	-I$(top_builddir)/config

libRedirectorJoin_la_SOURCES =  \
	SipRedirectorJoin.cpp \
	SipRedirectorJoin.h

# SipRedirectorPresenceRouting

libRedirectorPresenceRouting_la_LDFLAGS = \
    -module \
    -version-info ${version_Current}:${version_Revision}:${version_Age}

libRedirectorPresenceRouting_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@

libRedirectorPresenceRouting_la_CXXFLAGS = \
    -I$(top_builddir)/config

libRedirectorPresenceRouting_la_SOURCES =  \
    SipRedirectorPresenceRouting.cpp \
    SipRedirectorPresenceRouting.h

# SipRedirectorTimeOfDay

libRedirectorTimeOfDay_la_LDFLAGS = \
	-module \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libRedirectorTimeOfDay_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libRedirectorTimeOfDay_la_CXXFLAGS = \
	-I$(top_builddir)/config

libRedirectorTimeOfDay_la_SOURCES =  \
	SipRedirectorTimeOfDay.cpp \
	SipRedirectorTimeOfDay.h

# SipRedirectorUserParam

libRedirectorUserParam_la_LDFLAGS = \
	-module \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libRedirectorUserParam_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libRedirectorUserParam_la_CXXFLAGS = \
	-I$(top_builddir)/config

libRedirectorUserParam_la_SOURCES =  \
	SipRedirectorUserParam.cpp \
	SipRedirectorUserParam.h

# SipRedirectorTest
# Redirector to test request suspend/resume.

libRedirectorTest_la_LDFLAGS = \
	-module \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libRedirectorTest_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libRedirectorTest_la_CXXFLAGS = \
	-I$(top_builddir)/config \
	-DRES_PARSE_NAPTR

libRedirectorTest_la_SOURCES =  \
	SipRedirectorTest.cpp \
	SipRedirectorTest.h

# Incomplete redirectors

# # SipRedirectorGateway
# # Experimental redirector to allow configuring gateway routing prefixes.
#
# libRedirectorGateway_la_LDFLAGS = \
# 	-module \
# 	-version-info ${version_Current}:${version_Revision}:${version_Age}
#
# libRedirectorGateway_la_LIBADD = \
# 	@SIPXCOMMSERVER_LIBS@
#
# libRedirectorGateway_la_CXXFLAGS = \
# 	-I$(top_builddir)/config \
# 	-DRES_PARSE_NAPTR
#
# libRedirectorGateway_la_SOURCES =  \
# 	SipRedirectorGateway.cpp \
# 	SipRedirectorGateway.h
#
# # SipRedirectorMPT
# # Experimental redirector for multi-party testing.
#
# libRedirectorMPT_la_LDFLAGS = \
# 	-module \
# 	-version-info ${version_Current}:${version_Revision}:${version_Age}
#
# libRedirectorMPT_la_LIBADD = \
# 	@SIPXCOMMSERVER_LIBS@
#
# libRedirectorMPT_la_CXXFLAGS = \
# 	-I$(top_builddir)/config
#
# libRedirectorMPT_la_SOURCES =  \
# 	SipRedirectorMPT.cpp \
# 	SipRedirectorMPT.h
