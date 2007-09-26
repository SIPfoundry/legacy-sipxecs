#INCLUDES = -I$(top_srcdir)/src/cse -I$(top_srcdir)/src/sipauthproxy -I$(top_srcdir)/src/sipforkingproxy

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = proxytest enforcerulestest calleraliastest # transfercontroltest

check_PROGRAMS = \
	proxytest \
	calleraliastest \
	transfercontroltest \
	enforcerulestest 
# sandbox

COMMON_CXX_FLAGS = \
	-DTEST_DIR=\"@abs_builddir@\" \
	-DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test/\" \
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

proxytest_LDADD = \
	$(COMMON_LIBS) \
	$(top_builddir)/src//cse/libcse.la \
	$(top_builddir)/src/sipforkingproxy/libforkingproxy.la \
	$(top_builddir)/src/sipauthproxy/libauthproxy.la

enforcerulestest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

enforcerulestest_SOURCES = \
	$(top_srcdir)/src/sipauthproxy/EnforceAuthRules.cpp \
	EnforceAuthRulesTest.cpp

enforcerulestest_LDADD = \
	$(COMMON_LIBS) \
	$(top_builddir)/src/sipauthproxy/libauthproxy.la

calleraliastest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

calleraliastest_SOURCES = \
	$(top_srcdir)/src/sipauthproxy/CallerAlias.cpp \
	CallerAliasTest.cpp

calleraliastest_LDADD = \
	$(COMMON_LIBS) \
	$(top_builddir)/src/sipauthproxy/libauthproxy.la

transfercontroltest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

transfercontroltest_SOURCES = \
	$(top_srcdir)/src/sipauthproxy/TransferControl.cpp \
	TransferControlTest.cpp

transfercontroltest_LDADD = \
	$(COMMON_LIBS) \
	$(top_builddir)/src/sipauthproxy/libauthproxy.la

EXTRA_DATA = \
	authexception.xml \
	caller-alias.xml \
	credential.xml \
	permission.xml \
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

