## Process this file with automake to produce Makefile.in


SQAPLUGIN_COMMON_CXX_FLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
        -DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
        -DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
        -DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

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

SQAPLUGIN_COMMON_LD_FLAGS = \
    -module \
    -version-info ${version_Current}:${version_Revision}:${version_Age}

nobase_lib_LTLIBRARIES = \
    plugin/libsqaclient_plugin.la

plugin_libsqaclient_plugin_la_SOURCES = \
    sqaclient_plugin.cpp

plugin_libsqaclient_plugin_la_CXXFLAGS = \
    $(SQAPLUGIN_COMMON_CXX_FLAGS)

plugin_libsqaclient_plugin_la_LDFLAGS = \
    $(SQAPLUGIN_COMMON_LD_FLAGS)

plugin_libsqaclient_plugin_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@ \
	-lboost_date_time-mt \
	-lboost_filesystem-mt \
	-lboost_system-mt \
	-lboost_thread-mt \
	-lboost_program_options-mt \
	-lpthread \
	-lzmq \
	-lmongoclient \
	../libsipXsqa.la


