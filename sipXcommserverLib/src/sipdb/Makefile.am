## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/include/fastdb

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
	@ODBC_LDFLAGS@

libsipdb_la_CXXFLAGS = \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\"

libsipdb_la_LIBADD = \
	@SIPXTACK_LIBS@

libsipdb_la_SOURCES =  \
	AliasDB.cpp \
	CallerAliasDB.cpp \
   CredentialDB.cpp \
   DialByNameDB.cpp \
   ExtensionDB.cpp \
   LocationDB.cpp \
   UserLocationDB.cpp \
   UserForwardDB.cpp \
   UserStaticDB.cpp \
   RegistrationBinding.cpp \
   PermissionDB.cpp \
   RegistrationDB.cpp \
   ResultSet.cpp \
   SIPDBManager.cpp \
   SIPXAuthHelper.cpp \
   SubscriptionDB.cpp
