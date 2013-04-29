## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite

INCLUDES = -I$(top_srcdir)/include -I../

# Make TESTDIR an absolute path to the directory containing
# SipSrvLookupTest.named.*
TESTDIR=$(if $(patsubst /%,,$(top_srcdir)),$(shell pwd)/$(top_srcdir),$(top_srcdir))/src/test/net

# Only -D NAMED_PROGRAM if ./configure has found named and thus set
# @NAMED_PROGRAM@ to non-null.
testsuite_CXXFLAGS = \
    -DTESTDIR=\"$(TESTDIR)\" \
    $(if @NAMED_PROGRAM@,-DNAMED_PROGRAM=\"@NAMED_PROGRAM@\")

testsuite_LDADD = \
    @SIPXUNIT_LIBS@ \
    	@SIPXPORT_LIBS@ \
    	@SIPXCOMMSERVER_LIBS@ \
    ../libsipXsqa.la \
	  -lboost_date_time-mt \
	  -lboost_filesystem-mt \
	  -lboost_system-mt \
	  -lboost_thread-mt \
	  -lboost_program_options-mt \
	  -lpthread \
	  -lzmq \
	  -lmongoclient \
	  -lhiredis \
	  -lev

testsuite_SOURCES = \
    StateQueueConnectionTest.cpp

EXTRA_DIST =
