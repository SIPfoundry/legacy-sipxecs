INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/lib/redirect_plugins \
	-I$(top_srcdir)/include 


## All tests under this GNU variable should run relatively quickly
## and of course require no setup
# for performance numbers, add to TESTS: UtlListPerformance UtlHashMapPerformance
TESTS = testsuite

check_PROGRAMS = testsuite

testsuite_LDADD = \
	@SIPXUNIT_LIBS@ \
	@SIPXCOMMSERVER_LIBS@

testsuite_LDFLAGS = \
	-rdynamic

testsuite_CXXFLAGS = \
	@PCRE_CFLAGS@ \
	@CPPUNIT_CFLAGS@ \
	-DTEST_DIR=\"@abs_builddir@\" \
	-fno-gcse -O0

testsuite_SOURCES = \
    ../../../src/RedirectPlugin.cpp \
    ../../../src/RedirectSuspend.cpp \
    ../../../src/RedirectResumeMsg.cpp \
    ../../../src/SipRedirectServer.cpp \
	SipRedirectorTimeOfDayTest.cpp

