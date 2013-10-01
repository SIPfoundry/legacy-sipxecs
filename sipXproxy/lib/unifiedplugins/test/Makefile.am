## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = \
	plugintemplatetest
	
check_PROGRAMS = $(TESTS)

COMMON_CXX_FLAGS = \
	-DSIPX_LIBDIR=\"@SIPX_LIBDIR@\" \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/lib/unifiedplugins \
	-I$(top_srcdir)/src

COMMON_LIBS = \
	$(top_builddir)/src/libsipXproxy.la \
	@SIPXUNIT_LIBS@ \
	@SIPXCOMMSERVER_LIBS@ \
	-lmongoclient \
	-lboost_system-mt

plugintemplatetest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

plugintemplatetest_SOURCES = \
	WSRouterTest.cpp

plugintemplatetest_LDADD = \
	$(COMMON_LIBS)


