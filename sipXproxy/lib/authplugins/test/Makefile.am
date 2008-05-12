## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = enforcerulestest calleraliastest transfercontroltest msftxferhacktest

check_PROGRAMS = \
	calleraliastest \
	transfercontroltest \
	msftxferhacktest \
	enforcerulestest 
# sandbox

COMMON_CXX_FLAGS = \
	-DTEST_WORK_DIR=\"@abs_builddir@\" \
	-DTEST_DATA_DIR=\"@abs_top_srcdir@/lib/authplugins/test\" \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/lib/authplugins \
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

calleraliastest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

calleraliastest_SOURCES = \
	$(top_srcdir)/lib/authplugins/CallerAlias.cpp \
	CallerAliasTest.cpp

calleraliastest_LDADD = \
	$(COMMON_LIBS)

transfercontroltest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

transfercontroltest_SOURCES = \
	$(top_srcdir)/lib/authplugins/TransferControl.cpp \
	TransferControlTest.cpp

transfercontroltest_LDADD = \
	$(COMMON_LIBS)

msftxferhacktest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

msftxferhacktest_SOURCES = \
	$(top_srcdir)/lib/authplugins/MSFT_ExchangeTransferHack.cpp \
	MSFT_ExchangeTransferHackTest.cpp

msftxferhacktest_LDADD = \
	$(COMMON_LIBS)


EXTRA_DATA = \
	caller-alias.xml \
	credential.xml \
	permission.xml \
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

