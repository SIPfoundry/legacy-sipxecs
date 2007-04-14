## Process this file with automake to produce Makefile.in

FORKINGPROXY_COMMON_CXX_FLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/src/cse \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

bin_PROGRAMS = sipproxy

sipproxy_CXXFLAGS = \
	$(FORKINGPROXY_COMMON_CXX_FLAGS)

sipproxy_LDADD = \
	libforkingproxy.la \
	../cse/libcse.la \
	@SIPXCOMMSERVER_LIBS@

sipproxy_SOURCES =  \
	$(top_builddir)/config/sipxproxy-buildstamp.h \
	$(top_builddir)/config/sipxproxy-buildstamp.cpp \
	SipForkingProxyMain.cpp

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

## The objects in the forkingproxy are built as a libtool convenience library
## so that they can be more easily used in the unit tests in ../test
## The convenience library is static, and is never installed - it is statically 
## linked to create the executable above.
noinst_LTLIBRARIES = libforkingproxy.la

libforkingproxy_la_SOURCES = \
	SipRouter.cpp \
	ForwardRules.cpp \
	SipRouter.h \
	ForwardRules.h \
	ForkingProxyCseObserver.h \
	ForkingProxyCseObserver.cpp

libforkingproxy_la_CXXFLAGS = \
	$(FORKINGPROXY_COMMON_CXX_FLAGS)

