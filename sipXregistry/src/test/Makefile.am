INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/include 

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
# These tests are split into multiple executables because the machinery
# for setting up an alternative registration database can be executed
# only once in an executable.
#TESTS = testsuite1 testsuite2 testsuite3 testsuite4
TESTS = 

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
testsuite4_LDADD = $(testsuite_LDADD)

testsuite4_SOURCES = \
    ../RedirectPlugin.cpp \
    ../SipRedirectServer.cpp \
    ErrorDescriptorTest.cpp \
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
