## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include

noinst_LTLIBRARIES = libsipdb.la

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipdb_la_LDFLAGS = \
	-version-info ${version_Current}:${version_Revision}:${version_Age} \
	-shared \
	@ODBC_LDFLAGS@

# Fedora 17 port - Permissive compiler to get around 
#   MongoDB.cpp:68:1:   required from here
#   /usr/include/boost/program_options/detail/config_file.hpp:163:13: error: 'to_internal' was not
#   declared in this scope, and no declarations were found by argument-dependent lookup at the
#   point of instantiation [-fpermissive]
#
libsipdb_la_CXXFLAGS = \
	-fpermissive \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\"

libsipdb_la_LIBADD = \
	@SIPXTACK_LIBS@ \
	-lboost_system-mt

libsipdb_la_SOURCES =  \
   MongoDB.cpp \
   MongoOpLog.cpp \
   EntityDB.cpp \
   EntityRecord.cpp \
   RegBinding.cpp \
   RegDB.cpp \
   Subscription.cpp \
   SubscribeDB.cpp \
   ResultSet.cpp \
   DbHelper.cpp \
   CustomConnPool.cpp \
   unqlite.c \
   KeyValueStore.cpp
   
