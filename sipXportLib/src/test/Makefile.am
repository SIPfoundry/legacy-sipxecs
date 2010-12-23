
SUBDIRS = sipxunit testplugin

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
# for performance numbers, add to TESTS: UtlListPerformance UtlHashMapPerformance
TESTS = testsuite

check_PROGRAMS = testsuite sandbox UtlListPerformance UtlHashMapPerformance

## To load source in gdb for libsipXport.la, type the 'share' at the
## gdb console just before stepping into function in sipXportLib
testsuite_LDADD = \
    sipxunit/libsipXunit.la \
    ../libsipXport.la

testsuite_CXXFLAGS = \
	-I$(top_srcdir)/include \
	-I.. \
	@PCRE_CFLAGS@ \
	@SSL_CXXFLAGS@ \
	-DTEST_DIR=\"@abs_builddir@\" \
	-fno-gcse -O0

# ORDER NOTE: the utl/UtlLinkTest and utl/UtlChainTest must be before
#             any test that uses any UtlContainer.
testsuite_SOURCES = \
    utl/UtlChainTest.cpp \
    utl/UtlLinkTest.cpp \
    utl/UtlFsmTest.cpp \
    utl/UtlContainableTestStub.cpp \
    utl/UtlContainableTestStub.h \
    utl/UtlVoidPtr.cpp \
    utl/UtlInt.cpp \
    utl/UtlLongLongInt.cpp \
    utl/UtlStringTest.cpp \
    utl/UtlStringTest.h \
    utl/UtlStringTest_ConstructiveManipulators.cpp \
    utl/UtlStringTest_DestructiveManipulators.cpp \
    utl/UtlStringTest_NonMutating.cpp \
    utl/UtlSList.cpp \
    utl/UtlSListIteratorTest.cpp \
    utl/UtlDList.cpp \
    utl/UtlDListIterator.cpp \
    utl/UtlSortedList.cpp \
    utl/UtlSortedListIteratorTest.cpp \
    utl/UtlHashMap.cpp \
    utl/UtlHashMapIterator.cpp \
    utl/UtlHashBag.cpp \
    utl/UtlHashBagIterator.cpp \
    utl/UtlRegex.cpp \
    utl/UtlTokenizerTest.cpp \
    utl/XmlContentTest.cpp \
    os/OsTestUtilities.cpp \
    os/OsTestUtilities.h \
    os/OsNameDbTest.cpp \
    os/OsCallbackTest.cpp \
    os/OsTimerTaskTest.cpp \
    os/OsTimeTest.cpp \
    os/OsEventTest.cpp \
    os/OsLockTest.cpp \
    os/OsMsgQTest.cpp \
    os/OsMutexTest.cpp \
    os/OsSemTest.cpp \
    os/OsSocketTest.cpp \
    os/OsSysLogTest.cpp \
    os/OsConfigDbTest.cpp \
    os/OsFileIteratorTest.cpp \
    os/OsFileSystemTest.cpp \
    os/OsFileTest.cpp \
    os/OsPathTest.cpp \
    os/OsDirTest.cpp \
    os/OsProcessIteratorTest.cpp \
    os/OsProcessTest.cpp \
    os/OsTimerTest.cpp \
    os/OsUnLockTest.cpp \
    utl/TestPlugin.h \
    utl/PluginHooksTest.cpp

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
sandbox_LDADD = $(testsuite_LDADD)

sandbox_CXXFLAGS = $(testsuite_CXXFLAGS)
sandbox_SOURCES = \
	os/OsTaskTest.cpp

# Performance test of UtlList

UtlListPerformance_SOURCES = \
	utl/UtlPerformanceStrings.h \
	utl/UtlListPerformance.cpp


UtlListPerformance_CXXFLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include

UtlListPerformance_LDADD = \
    ../libsipXport.la

# Performance test of UtlHashMap

UtlHashMapPerformance_SOURCES = \
	utl/UtlPerformanceStrings.h \
	utl/UtlHashMapPerformance.cpp


UtlHashMapPerformance_CXXFLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include

UtlHashMapPerformance_LDADD = \
    ../libsipXport.la

EXTRA_DIST=

DISTCLEANFILES = Makefile.in
