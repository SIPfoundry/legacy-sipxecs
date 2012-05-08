## Process this file with automake to produce Makefile.in

TRANSACTIONPLUGIN_COMMON_CXX_FLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

TRANSACTIONPLUGIN_COMMON_LD_FLAGS = \
    -module \
    -version-info ${version_Current}:${version_Revision}:${version_Age}

nobase_lib_LTLIBRARIES = \
    transactionplugins/libSipHeaderCheck.la

transactionplugins_libSipHeaderCheck_la_SOURCES = \
    HeaderCheck.h \
    HeaderCheck.cpp

transactionplugins_libSipHeaderCheck_la_CXXFLAGS = \
    $(TRANSACTIONPLUGIN_COMMON_CXX_FLAGS)

transactionplugins_libSipHeaderCheck_la_LDFLAGS = \
    $(TRANSACTIONPLUGIN_COMMON_LD_FLAGS)

transactionplugins_libSipHeaderCheck_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@


