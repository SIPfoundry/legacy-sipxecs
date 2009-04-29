
TESTS = testsuite

check_PROGRAMS = \
	testsuite

COMMON_CXX_FLAGS = \
	-DTEST_WORK_DIR=\"@abs_builddir@/work\" \
	-DTEST_DATA_DIR=\"@abs_top_srcdir@/sipXrls/src/test/rlsdata\" \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/lib/authplugins

COMMON_LIBS = \
   @SIPXUNIT_LIBS@ \
   @SIPXCOMMSERVERTEST_LIBS@ \
   @SIPXCOMMSERVER_LIBS@ 

testsuite_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

testsuite_SOURCES = \
    ../ContactSet.cpp \
    ../ResourceCache.cpp \
    ../ResourceCached.cpp \
    ../ResourceInstance.cpp \
    ../ResourceList.cpp \
    ../ResourceListFileReader.cpp \
    ../ResourceListMsg.cpp \
    ../ResourceListServer.cpp \
    ../ResourceListSet.cpp \
    ../ResourceListTask.cpp \
    ../ResourceNotifyReceiver.cpp \
    ../ResourceReference.cpp \
    ../ResourceSubscriptionReceiver.cpp \
    ../RlsSubscribePolicy.cpp \
    ../SubscriptionSet.cpp \
    RlsTestFixtures.cpp \
    ResourceListServerTest.cpp


testsuite_LDADD = \
	$(COMMON_LIBS)

#EXTRA_DIST = \
#  cse_test.rb \
#  environment.rb \
#  sipsupport.rb

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

#sandbox_LDADD = $(testsuite_LDADD)

#sandbox_CXXFLAGS = $(testsuite_CXXFLAGS)
#sandbox_SOURCES = \
#   ResourceListServerTest.cpp
