
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
   @SIPXCOMMSERVER_LIBS@

testsuite_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

testsuite_SOURCES = \
    ../AppearanceAgent.cpp \
    ../Appearance.cpp \
    ../AppearanceGroup.cpp \
    ../AppearanceGroupSet.cpp \
    ../AppAgentSubscribePolicy.cpp \
    ../AppAgentTask.cpp \
    ../ResourceListMsg.cpp \
    ../ResourceSubscriptionReceiver.cpp \
    ../ResourceNotifyReceiver.cpp
# disabled for mongo port
#    RlsTestFixtures.cpp
#    AppearanceAgentTest.cpp


testsuite_LDADD = \
	$(COMMON_LIBS)
