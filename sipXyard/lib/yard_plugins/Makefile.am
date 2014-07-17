## Process this file with automake to produce Makefile.in

SUBDIRS = . 

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

YARDPLUGIN_COMMON_CXX_FLAGS = \
	-DSIPXYARD_PLUGIN_DIR=\"@SIPXYARD_PLUGIN_DIR@\" \
	-I$(top_srcdir)/include
	
YARDPLUGIN_COMMON_LD_FLAGS = \
    -module \
    -version-info ${version_Current}:${version_Revision}:${version_Age}

YARDPLUGIN_COMMON_LIBADD = \
    @SIPXCOMMSERVER_LIBS@ \
    @SIPXYARD_LIBS@

nobase_lib_LTLIBRARIES = \
    yard_plugins/libConfigDumper.la \
    yard_plugins/libProcessControl.la

yard_plugins_libConfigDumper_la_SOURCES = \
    ConfigDumper.h \
    ConfigDumper.cpp

yard_plugins_libConfigDumper_la_CXXFLAGS = \
    $(YARDPLUGIN_COMMON_CXX_FLAGS)

yard_plugins_libConfigDumper_la_LDFLAGS = \
    $(YARDPLUGIN_COMMON_LD_FLAGS)

yard_plugins_libConfigDumper_la_LIBADD = \
    $(YARDPLUGIN_COMMON_LIBADD)

yard_plugins_libProcessControl_la_SOURCES = \
    ProcessControl.h \
    ProcessControl.cpp

yard_plugins_libProcessControl_la_CXXFLAGS = \
    $(YARDPLUGIN_COMMON_CXX_FLAGS)

yard_plugins_libProcessControl_la_LDFLAGS = \
    $(YARDPLUGIN_COMMON_LD_FLAGS)

yard_plugins_libProcessControl_la_LIBADD = \
    $(YARDPLUGIN_COMMON_LIBADD)


