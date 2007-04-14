
SUBDIRS = test

lib_LTLIBRARIES = libRegistrarImpliedMWI.la

INCLUDES = -I$(top_srcdir)/include

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libRegistrarImpliedMWI_la_LDFLAGS = \
	-module \
	-version-info ${version_Current}:${version_Revision}:${version_Age} \
	@PCRE_LDFLAGS@ \
	@SSL_LDFLAGS@

libRegistrarImpliedMWI_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libRegistrarImpliedMWI_la_CXXFLAGS = \
	@PCRE_CFLAGS@ \
	-I$(top_builddir)/config

libRegistrarImpliedMWI_la_SOURCES =  \
   SipImpliedSubscriptions.cpp \
	SipImpliedSubscriptions.h
