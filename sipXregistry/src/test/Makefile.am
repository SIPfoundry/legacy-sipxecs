INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/include

noinst_LTLIBRARIES = \
    libDummyRedirectPlugin.la

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

# DummyRedirectPlugin

libDummyRedirectPlugin_la_LDFLAGS = \
    -module \
    -rpath @abs_builddir@ \
    -version-info ${version_Current}:${version_Revision}:${version_Age}

libDummyRedirectPlugin_la_LIBADD = \
    @SIPXUNIT_LIBS@

libDummyRedirectPlugin_la_CXXFLAGS = \
    -I$(top_builddir)/config

libDummyRedirectPlugin_la_SOURCES =  \
    DummyRedirectPlugin.cpp


## All tests under this GNU variable should run relatively quickly
## and of course require no setup
# These tests are split into multiple executables because the machinery
# for setting up an alternative registration database can be executed
# only once in an executable.
#TESTS = testsuite1 testsuite2 testsuite3 testsuite4
TESTS = testsuite4

check_PROGRAMS = testsuite1 testsuite2 testsuite3 testsuite4 sandbox

testsuite_CXXFLAGS = @CPPUNIT_CFLAGS@ \
    -DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test/\" \
    -DTEST_WORK_DIR=\"@abs_top_builddir@/src/test/work\"

testsuite_LDADD = \
   @SIPXCOMMSERVERTEST_LIBS@ \
   ../libsipXregistry.la

testsuite1_CXXFLAGS = $(testsuite_CXXFLAGS)
testsuite1_LDADD = $(testsuite_LDADD)

testsuite1_SOURCES = \
    RegistrarPeerTest.cpp

testsuite2_CXXFLAGS = $(testsuite_CXXFLAGS)
testsuite2_LDADD = $(testsuite_LDADD)

testsuite2_SOURCES = \
    SipRegistrarServerTest.cpp

testsuite3_CXXFLAGS = $(testsuite_CXXFLAGS)
testsuite3_LDADD = $(testsuite_LDADD)

testsuite3_SOURCES = \
    ApplyUpdatesTest.cpp

testsuite4_CXXFLAGS = $(testsuite_CXXFLAGS)
testsuite4_LDFLAGS = -rdynamic
testsuite4_LDADD = $(testsuite_LDADD)

testsuite4_SOURCES = \
    ../RedirectPlugin.cpp \
    ../SipRedirectServer.cpp \
    ../RedirectSuspend.cpp \
    ../RedirectResumeMsg.cpp \
    ErrorDescriptorTest.cpp \
    ContactListTest.cpp \
    SipRedirectServerTest.cpp

EXTRA_DIST = \
    regdbdata/updatesToPull.xml

# [DEBUG ASSISTANT] List a single unit test to isolate a unit test for
# debugging or when developing a new unit test. As courtesy remove all
# unit tests before checking in, although forgetting to has no ill
# effects. To build and run:
#
#   cd src/test
#   make sandbox
#   ./sandbox
#
#sandbox_LDFLAGS = $(testsuite_LDFLAGS)
sandbox_LDADD = $(testsuite_LDADD)

sandbox_CXXFLAGS = $(testsuite_CXXFLAGS)
sandbox_SOURCES = \
    SipRegistrarServerTest.cpp
