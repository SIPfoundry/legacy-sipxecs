INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/lib/redirect_plugins \
	-I$(top_srcdir)/include


## All tests under this GNU variable should run relatively quickly
## and of course require no setup
# for performance numbers, add to TESTS: UtlListPerformance UtlHashMapPerformance
TESTS = testsuite1 testsuite2 testsuite3

check_PROGRAMS = testsuite1 testsuite2 testsuite3

testsuite1_LDADD = \
    @SIPXUNIT_LIBS@ \
    @SIPXCOMMSERVER_LIBS@

testsuite1_LDFLAGS = \
    -rdynamic

testsuite1_CXXFLAGS = \
    @PCRE_CFLAGS@ \
    @CPPUNIT_CFLAGS@ \
    -DTEST_DIR=\"@abs_builddir@\" \
    -fno-gcse -O0

testsuite1_SOURCES = \
    ../../../src/RedirectPlugin.cpp \
    ../../../src/RedirectSuspend.cpp \
    ../../../src/RedirectResumeMsg.cpp \
    ../../../src/SipRedirectServer.cpp \
    SipRedirectorTimeOfDayTest.cpp

testsuite2_LDADD = \
    @SIPXUNIT_LIBS@ \
    @SIPXCOMMSERVERTEST_LIBS@ \
    @SIPXCOMMSERVER_LIBS@

testsuite2_LDFLAGS = \
    -rdynamic

testsuite2_CXXFLAGS = @CPPUNIT_CFLAGS@ \
    -DTEST_DATA_DIR=\"@abs_top_srcdir@/lib/redirect_plugins/test/locationdbdata\" \
    -DTEST_WORK_DIR=\"@abs_top_builddir@/lib/redirect_plugins/test/work\"
testsuite2_SOURCES = \
    ../../../src/RedirectPlugin.cpp \
    ../../../src/RedirectSuspend.cpp \
    ../../../src/RedirectResumeMsg.cpp \
    ../../../src/SipRedirectServer.cpp \
    ../SipRedirectorFallback.cpp \
    SipRedirectorFallbackTest.cpp

testsuite3_LDADD = \
    @SIPXUNIT_LIBS@ \
    @SIPXCOMMSERVERTEST_LIBS@ \
    @SIPXCOMMSERVER_LIBS@

testsuite3_LDFLAGS = \
    -rdynamic

testsuite3_CXXFLAGS = @CPPUNIT_CFLAGS@ \
    -DTEST_DATA_DIR=\"@abs_top_srcdir@/lib/redirect_plugins/test/locationdbdata\" \
    -DTEST_WORK_DIR=\"@abs_top_builddir@/lib/redirect_plugins/test/work\"
testsuite3_SOURCES = \
    ../../../src/RedirectPlugin.cpp \
    ../../../src/RedirectSuspend.cpp \
    ../../../src/RedirectResumeMsg.cpp \
    ../../../src/SipRedirectServer.cpp \
    ../SipRedirectorPresenceRouting.cpp \
    SipRedirectorPresenceRoutingTest.cpp
