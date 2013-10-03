## Process this file with automake to produce Makefile.in

SUBDIRS = .

INCLUDES = \
	-I$(top_srcdir)/include

lib_LTLIBRARIES = libsipXrepro.la

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=2
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXrepro_la_LDFLAGS = \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libsipXrepro_la_CXXFLAGS = \
	-g -O0 \
	@PCRE_CXXFLAGS@ \
	@SSL_CXXFLAGS@ \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

# resparse uses .c files, so we need to define the CFLAGS also.
libsipXrepro_la_CFLAGS = \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" 

libsipXrepro_la_LIBADD = \
	@SIPXPORT_LIBS@ \
	-lrepro \
	-ldum \
	-lresip \
	-lrutil \
	-lstdc++

libsipXrepro_la_SOURCES =  \
    ReproGlue.cpp \
    SipxRepro.cpp