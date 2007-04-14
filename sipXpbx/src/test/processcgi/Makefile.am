## All tests under this GNU variable should run relatively quickly
## and of course require no setup
# for performance numbers, add to TESTS: UtlListPerformance UtlHashMapPerformance
TESTS = testsuite

check_PROGRAMS = testsuite

## To load source in gdb for libsipXport.la, type the 'share' at the
## gdb console just before stepping into function in sipXportLib
testsuite_LDADD = \
    -lsipXunit \
    -lsipXport \
    @SSL_LDFLAGS@

testsuite_CXXFLAGS = \
	-I$(top_srcdir)/include \
	@CPPUNIT_CFLAGS@ \
	-DTEST_SRC_DIR=\"@abs_srcdir@\" \
	-DTEST_DIR=\"@abs_builddir@\" \
	-DSIPX_TMPDIR=\"@abs_builddir@\" \
	-O0

if ISGCC
testsuite_CXXFLAGS += -fno-gcse
endif

testsuite_SOURCES = \
    ../../processcgi/processCommon.cpp \
    ../../processcgi/DependencyList.cpp \
    WatchdogProcessXmlTest.cpp

# [DEBUG ASSISTANT] List a single unittest to isoloate a unittest for
# debugging or when developing a new unittest. As courtesy remove all
# unittests before checking in, although forgetting to has no ill
# effects. To build and run:
#
#   cd src/test
#   make sandbox
#   ./sandbox
#
#sandbox_LDFLAGS = $(testsuite_LDFLAGS)
#sandbox_LDADD = $(testsuite_LDADD)

#sandbox_CXXFLAGS = $(testsuite_CXXFLAGS)
#sandbox_SOURCES =

DISTCLEANFILES = Makefile.in
