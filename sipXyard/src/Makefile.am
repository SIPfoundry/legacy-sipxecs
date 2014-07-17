SIPXYARD_COMMON_CXX_FLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/include \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPXYARD_PLUGIN_DIR=\"@SIPXYARD_PLUGIN_DIR@\"

bin_PROGRAMS = sipxyard

sipxyard_CXXFLAGS = \
	$(SIPXYARD_COMMON_CXX_FLAGS)

sipxyard_LDFLAGS = \
    -shared \
    -rdynamic

sipxyard_LDADD = \
	@SIPXCOMMSERVER_LIBS@ \
	libsipXyard.la

sipxyard_SOURCES =  \
	sipXyard.cpp


## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

lib_LTLIBRARIES = \
    libsipXyard.la

libsipXyard_la_LDFLAGS = \
    -shared \
    -version-info ${version_Current}:${version_Revision}:${version_Age}

libsipXyard_la_SOURCES = \
	ProcessInfo.cpp \
	RESTClient.cpp \
	RESTServer.cpp \
	LevelDB.cpp \
	YardUtils.cpp \
	YardPlugin.cpp

libsipXyard_la_CXXFLAGS = \
	$(SIPXYARD_COMMON_CXX_FLAGS)
	
libsipXyard_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@ \
	-lmongoclient \
	-lleveldb \
	-lboost_program_options-mt \
	-lPocoFoundation \
	-lPocoUtil \
	-lPocoNet \
	-lPocoNetSSL \
	-lPocoXML \
	-lleveldb


nobase_lib_LTLIBRARIES = \
    yard_plugins/libConfigDumper.la \
    yard_plugins/libProcessControl.la

YARDPLUGIN_COMMON_CXX_FLAGS = \
	-DSIPXYARD_PLUGIN_DIR=\"@SIPXYARD_PLUGIN_DIR@\" \
	-I$(top_srcdir)/include
	
YARDPLUGIN_COMMON_LD_FLAGS = \
    -module \
    -version-info ${version_Current}:${version_Revision}:${version_Age}

YARDPLUGIN_COMMON_LIBADD = \
    @SIPXCOMMSERVER_LIBS@ \
    libsipXyard.la

yard_plugins_libConfigDumper_la_SOURCES = \
    lib/yard_plugins/ConfigDumper.h \
    lib/yard_plugins/ConfigDumper.cpp

yard_plugins_libConfigDumper_la_CXXFLAGS = \
    $(YARDPLUGIN_COMMON_CXX_FLAGS)

yard_plugins_libConfigDumper_la_LDFLAGS = \
    $(YARDPLUGIN_COMMON_LD_FLAGS)

yard_plugins_libConfigDumper_la_LIBADD = \
    $(YARDPLUGIN_COMMON_LIBADD)

yard_plugins_libProcessControl_la_SOURCES = \
    lib/yard_plugins/ProcessControl.h \
    lib/yard_plugins/ProcessControl.cpp

yard_plugins_libProcessControl_la_CXXFLAGS = \
    $(YARDPLUGIN_COMMON_CXX_FLAGS)

yard_plugins_libProcessControl_la_LDFLAGS = \
    $(YARDPLUGIN_COMMON_LD_FLAGS)

yard_plugins_libProcessControl_la_LIBADD = \
    $(YARDPLUGIN_COMMON_LIBADD)

