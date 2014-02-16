## Process this file with automake to produce Makefile.in

SUBDIRS = \
	. \
	plugin \
	swig \
	test

## Process this file with automake to produce Makefile.in

SIPXSQA_COMMON_CXX_FLAGS = \
	-I$(top_srcdir)/include \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
        -DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
        -DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
        -DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

bin_PROGRAMS = sipxsqa

sipxsqa_CXXFLAGS = \
	$(SIPXSQA_COMMON_CXX_FLAGS)

sipxsqa_LDFLAGS = \
    -shared \
    -rdynamic

sipxsqa_LDADD = \
	libsipXsqa.la \
	@SIPXPORT_LIBS@ \
	@SIPXCOMMSERVER_LIBS@ \
	  -lboost_date_time-mt \
	  -lboost_filesystem-mt \
	  -lboost_system-mt \
	  -lboost_thread-mt \
	  -lboost_program_options-mt \
	  -lpthread \
	  -lzmq \
	  -lmongoclient \
	  -lhiredis \
	  -lev

sipxsqa_SOURCES =   main.cpp

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

lib_LTLIBRARIES = libsipXsqa.la

libsipXsqa_la_LDFLAGS = \
    -shared \
    -version-info ${version_Current}:${version_Revision}:${version_Age}

libsipXsqa_la_SOURCES =  \
	StateQueueAgent.cpp \
	StateQueueConnection.cpp \
	StateQueueDriverTest.cpp \
	StateQueueListener.cpp \
	StateQueuePersistence.cpp \
	StateQueuePublisher.cpp

libsipXsqa_la_CXXFLAGS = \
	$(SIPXSQA_COMMON_CXX_FLAGS)
	
libsipXsqa_la_CFLAGS = \
	-I$(top_srcdir)/include/sqa
