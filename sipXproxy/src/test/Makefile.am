#INCLUDES = -I$(top_srcdir)/src/cse -I$(top_srcdir)/src/sipXproxy

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = proxytest enforcerulestest calleraliastest transfercontroltest msftxferhacktest

check_PROGRAMS = \
	proxytest \
	calleraliastest \
	transfercontroltest \
	msftxferhacktest \
	enforcerulestest 
# sandbox

COMMON_CXX_FLAGS = \
	-DTEST_WORK_DIR=\"@abs_builddir@\" \
	-DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-I$(top_srcdir)/src

COMMON_LIBS = \
   @SIPXUNIT_LIBS@ \
   @SIPXCOMMSERVERTEST_LIBS@ \
   @SIPXCOMMSERVER_LIBS@

proxytest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

proxytest_SOURCES = \
   ForwardRulesTest.cpp \
   CallStateEventBuilder_XMLTest.cpp \
   CallStateEventBuilder_DBTest.cpp \
   RouteStateTest.cpp \
   SipRouterTest.cpp 

proxytest_LDFLAGS = \
    -rdynamic

proxytest_LDADD = \
	$(COMMON_LIBS) \
	$(top_builddir)/src//cse/libcse.la \
	$(top_builddir)/src/sipXproxy/libsipXproxy.la 

enforcerulestest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

enforcerulestest_SOURCES = \
	$(top_srcdir)/src/sipXproxy/EnforceAuthRules.cpp \
	EnforceAuthRulesTest.cpp

enforcerulestest_LDADD = \
	$(COMMON_LIBS) \
	$(top_builddir)/src/sipXproxy/libsipXproxy.la

calleraliastest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

calleraliastest_SOURCES = \
	$(top_srcdir)/src/sipXproxy/CallerAlias.cpp \
	CallerAliasTest.cpp

calleraliastest_LDADD = \
	$(COMMON_LIBS) \
	$(top_builddir)/src/sipXproxy/libsipXproxy.la

transfercontroltest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

transfercontroltest_SOURCES = \
	$(top_srcdir)/src/sipXproxy/TransferControl.cpp \
	TransferControlTest.cpp

transfercontroltest_LDADD = \
	$(COMMON_LIBS) \
	$(top_builddir)/src/sipXproxy/libsipXproxy.la

msftxferhacktest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

msftxferhacktest_SOURCES = \
	$(top_srcdir)/src/sipXproxy/MSFT_ExchangeTransferHack.cpp \
	MSFT_ExchangeTransferHackTest.cpp

msftxferhacktest_LDADD = \
	$(COMMON_LIBS) \
	$(top_builddir)/src/sipXproxy/libsipXproxy.la

EXTRA_DATA = \
	authexception.xml \
	caller-alias.xml \
	credential.xml \
	permission.xml \
	transfer-control/domain-config \
   rulesdata/simple.xml \
   rulesdata/routing.xml


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

