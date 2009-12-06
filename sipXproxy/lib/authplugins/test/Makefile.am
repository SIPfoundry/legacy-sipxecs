## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = \
	calleraliastest \
	calldestinationtest \
	enforcerulestest \
	emergencyrulestest \
	requestlintertest \
	enforcerulestest \
	msftxferhacktest \
	nattraversalagenttests \
	nattraversaltests \
	transfercontroltest \
	subscriptionauthtest

check_PROGRAMS = \
	calleraliastest \
	calldestinationtest \
	enforcerulestest \
	emergencyrulestest \
	requestlintertest \
	msftxferhacktest \
	nattraversalagenttests \
	nattraversaltests \
	transfercontroltest \
	subscriptionauthtest
# sandbox

COMMON_CXX_FLAGS = \
	-DTEST_WORK_DIR=\"@abs_builddir@/work\" \
	-DTEST_DATA_DIR=\"@abs_top_srcdir@/lib/authplugins/test\" \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/lib/authplugins \
	-I$(top_srcdir)/lib/authplugins/NatTraversalAgent \
	-I$(top_srcdir)/src

COMMON_LIBS = \
	$(top_builddir)/src/libsipXproxy.la \
	@SIPXUNIT_LIBS@ \
	@SIPXCOMMSERVERTEST_LIBS@ \
	@SIPXCOMMSERVER_LIBS@

enforcerulestest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

enforcerulestest_SOURCES = \
	$(top_srcdir)/lib/authplugins/EnforceAuthRules.cpp \
	EnforceAuthRulesTest.cpp

enforcerulestest_LDADD = \
	$(COMMON_LIBS)

emergencyrulestest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

emergencyrulestest_SOURCES = \
	$(top_srcdir)/lib/authplugins/EmergencyNotify.cpp \
	EmergencyNotificationTest.cpp

emergencyrulestest_LDADD = \
	$(COMMON_LIBS)

requestlintertest_CXXFLAGS = \
    $(COMMON_CXX_FLAGS)

requestlintertest_SOURCES = \
    $(top_srcdir)/lib/authplugins/RequestLinter.cpp \
    RequestLinterTest.cpp

requestlintertest_LDADD = \
    $(COMMON_LIBS)

calleraliastest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

calleraliastest_SOURCES = \
	$(top_srcdir)/lib/authplugins/CallerAlias.cpp \
	CallerAliasTest.cpp

calleraliastest_LDADD = \
	$(COMMON_LIBS)

calldestinationtest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

calldestinationtest_SOURCES = \
	$(top_srcdir)/lib/authplugins/CallDestination.cpp \
	CallDestinationTest.cpp

calldestinationtest_LDADD = \
	$(COMMON_LIBS)

transfercontroltest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

transfercontroltest_SOURCES = \
	$(top_srcdir)/lib/authplugins/TransferControl.cpp \
	TransferControlTest.cpp

transfercontroltest_LDADD = \
	$(COMMON_LIBS)

nattraversaltests_CXXFLAGS = \
    $(COMMON_CXX_FLAGS)

nattraversaltests_SOURCES = \
    NatTraversalRulesTest.cpp \
    NatTraversalAgentDataTypesTest.cpp \
    SessionContextTest.cpp \
    DialogTrackerTest.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/NatTraversalRules.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/CallTracker.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/NatMaintainer.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/SessionContext.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/DialogTracker.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/DialogTrackerStates.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/NatTraversalAgentDataTypes.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/MediaRelay.cpp

nattraversaltests_LDADD = \
    $(COMMON_LIBS)

nattraversalagenttests_CXXFLAGS = \
    $(COMMON_CXX_FLAGS) -D_nat_unit_tests_

nattraversalagenttests_SOURCES = \
    NatTraversalAgentTest.cpp \
    DummyMediaRelay.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/NatTraversalRules.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/CallTracker.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/SessionContext.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/DialogTracker.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/DialogTrackerStates.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/NatTraversalAgentDataTypes.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/NatTraversalAgent.cpp \
    $(top_srcdir)/lib/authplugins/NatTraversalAgent/NatMaintainer.cpp

nattraversalagenttests_LDADD = \
    $(COMMON_LIBS)

msftxferhacktest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

msftxferhacktest_SOURCES = \
	$(top_srcdir)/lib/authplugins/MSFT_ExchangeTransferHack.cpp \
	MSFT_ExchangeTransferHackTest.cpp

msftxferhacktest_LDADD = \
	$(COMMON_LIBS)

subscriptionauthtest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

subscriptionauthtest_SOURCES = \
	$(top_srcdir)/lib/authplugins/SubscriptionAuth.cpp \
	SubscriptionAuthTest.cpp

subscriptionauthtest_LDADD = \
	$(COMMON_LIBS)

EXTRA_DATA = \
	credential.xml \
	permission.xml \
	example.edu/caller-aliase.xml \
	example.edu/domain-config \
	mydomain.com/domain-config \
	transfer-control/domain-config

EXTRA_DIST = $(EXTRA_DATA)

# [DEBUG ASSISTANT] List a single unit test to isolate a unit test for
# debugging or when developing a new unit test. As courtesy remove all
# unit tests before checking in, although forgetting to has no ill
# effects. To build and run:
#
#   cd src/test
#   make sandbox
#   ./sandbox
#
#sandbox_LDFLAGS = $(proxytest_LDFLAGS)

#sandbox_LDADD = $(proxytest_LDADD)

#sandbox_CXXFLAGS = $(proxytest_CXXFLAGS)
#sandbox_SOURCES = \
#   SipRouterTest.cpp
