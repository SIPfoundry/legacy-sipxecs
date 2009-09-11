
## All tests under this GNU variable should run relatively quickly
## and of course require no setup
# for performance numbers, add to TESTS: UtlListPerformance UtlHashMapPerformance
TESTS = testsuite

check_PROGRAMS = testsuite

testsuite_LDADD = \
	@SIPXUNIT_LIBS@ \
	@SIPXCOMMSERVER_LIBS@


testsuite_CXXFLAGS = \
	-I$(top_srcdir)/include \
	@PCRE_CFLAGS@ \
	@CPPUNIT_CFLAGS@ \
	-DTEST_DIR=\"@abs_builddir@\" \
	-fno-gcse -O0

# ORDER NOTE: the utl/UtlLinkTest and utl/UtlChainTest must be before
#             any test that uses any UtlContainer.
testsuite_SOURCES = \
	SipImpliedSubscriptionsTest.cpp
