INCLUDES = -I$(top_srcdir)/include

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite

testsuite_LDADD = \
    @SIPXUNIT_LIBS@ \
    ../../libsipXcall.la \
    @SIPXMEDIAADAPTER_LIBS@ \
    @GIPS_NEQ_OBJS@

testsuite_SOURCES = \
  callbacks.h \
  callbacks.cpp \
  EventRecorder.h \
  EventRecorder.cpp \
  EventValidator.h \
  EventValidator.cpp \
  sipXtapiTest.h \
  sipXtapiTest.cpp \
  sipXtapiTestCall.cpp \
  sipXtapiTestConference.cpp \
  sipXtapiTestConferenceAdv.cpp \
  sipXtapiTestConfig.cpp \
  sipXtapiTestLine.cpp \
  sipXtapiTestTransfer.cpp \
  TestRegistrar.h \
  TestRegistrar.cpp \
  TestRegistryCollection.h \
  TestRegistryCollection.cpp \
  TestRegistryData.h \
  TestRegistryData.cpp \
  ../jnibutton.cpp

DISTCLEANFILES = Makefile.in
