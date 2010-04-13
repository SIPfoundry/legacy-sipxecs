## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/interface \
	-I$(top_srcdir)/sipXmediaMediaProcessing

lib_LTLIBRARIES = libsipXmediaProcessing.la

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXmediaProcessing_la_LDFLAGS = -version-info ${version_Current}:${version_Revision}:${version_Age}

libsipXmediaProcessing_la_CXXFLAGS = \
	@SSL_CXXFLAGS@ \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

libsipXmediaProcessing_la_LIBADD = \
	../../interface/libmediainterface.la \
	@SIPXPORT_LIBS@ \
	@SSL_LIBS@

libsipXmediaProcessing_la_SOURCES =  \
    sipXmediaFactoryImpl.cpp \
    CpPhoneMediaInterface.cpp
