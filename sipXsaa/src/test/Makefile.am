
TESTS = testsuite

check_PROGRAMS = \
	testsuite

COMMON_CXX_FLAGS = \
	-DTEST_WORK_DIR=\"@abs_builddir@/work\" \
	-DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test/saadata\" \
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
    ../AppearanceAgent.cpp \
    ../Appearance.cpp \
    ../AppearanceGroup.cpp \
    ../AppearanceGroupFileReader.cpp \
    ../AppearanceGroupSet.cpp \
    ../AppAgentSubscribePolicy.cpp \
    ../AppAgentTask.cpp \
    ../ResourceListMsg.cpp \
    ../ResourceSubscriptionReceiver.cpp \
    ../ResourceNotifyReceiver.cpp \
    RlsTestFixtures.cpp \
    AppearanceAgentTest.cpp


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
#   AppearanceAgentTest.cpp
