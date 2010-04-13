INCLUDES = -I$(top_srcdir)/include

SUBDIRS = tapi

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite

testsuite_LDADD = \
    @SIPXUNIT_LIBS@ \
    ../libsipXcall.la \
    @SIPXMEDIAADAPTER_LIBS@ \
	@GIPS_NEQ_OBJS@

testsuite_SOURCES = \
    jnibutton.cpp \
    cp/CpTestSupport.cpp \
    cp/CpTestSupport.h \
    cp/CallManagerTest.cpp \
    ptapi/PtAddressForwardingTest.cpp \
    ptapi/PtAddressTest.cpp \
    ptapi/PtAudioCodecTest.cpp \
    ptapi/PtCallEventTest.cpp \
    ptapi/PtCallTest.cpp \
    ptapi/PtConnectionEventTest.cpp \
    ptapi/PtConnectionTest.cpp \
    ptapi/PtEventTest.cpp \
    ptapi/PtMediaCapabilities.cpp \
    ptapi/PtPhoneTerminalTest.cpp \
    ptapi/PtTerminalTest.cpp

## Full regression test and not meant for 'make check'
noinst_PROGRAMS = regression

regression_LDADD = \
    @SIPXUNIT_LIBS@ \
    ../libsipXcall.la \
    @SIPXMEDIAADAPTER_LIBS@ \
	@GIPS_NEQ_OBJS@

regression_SOURCES = \
    jnibutton.cpp \
    mp/CallManagerPlayerTest.cpp \
    mp/MpStreamPlayerTest.cpp \
    mp/MpStreamPlaylistPlayerTest.cpp \
    mp/MpStreamQueuePlayerTest.cpp \
    mp/MpTestConfig.cpp \
    mp/MyPlayerListenerHistoryKeeper.cpp \
    mp/MyPlayerListenerPoller.cpp \
    mp/MyStreamQueueHistoryKeeper.cpp

DISTCLEANFILES = Makefile.in

# [DEBUG ASSISTANT] List a single unittest to isolate a unittest for
# debugging or when developing a new unittest. As courtesy remove all
# unittests before checking in, although forgetting to has no ill
# effects.
# sandbox_LDADD = $(testsuite_LDADD)
# sandbox_CXXFLAGS = $(testsuite_CXXFLAGS)
# sandbox_SOURCES =
