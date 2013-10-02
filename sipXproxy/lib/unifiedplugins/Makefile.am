## Process this file with automake to produce Makefile.in

SUBDIRS = . test

UNIFIED_COMMON_CXX_FLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include \
	-DSIPX_LIBDIR=\"@SIPX_LIBDIR@\"

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if -lmongoclient \
    -lboost_system-mtany implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

UNIFIED_COMMON_LD_FLAGS = \
    -module \
    -version-info ${version_Current}:${version_Revision}:${version_Age}

nobase_lib_LTLIBRARIES = \
    unifiedplugins/libsipXwsRouter.la
    

unifiedplugins_libsipXwsRouter_la_SOURCES = \
    WSRouter.h \
    WSRouter.cpp

unifiedplugins_libsipXwsRouter_la_CXXFLAGS = \
    $(UNIFIED_COMMON_CXX_FLAGS)

unifiedplugins_libsipXwsRouter_la_LDFLAGS = \
    $(UNIFIED_COMMON_LD_FLAGS)

unifiedplugins_libsipXwsRouter_la_LIBADD = \
    @SIPXCOMMSERVER_LIBS@ \
    -lsipXrepro \
    -lrepro \
    -ldum \
    -lresip \
    -lrutil \
    -lmongoclient \
    -lboost_system-mt


