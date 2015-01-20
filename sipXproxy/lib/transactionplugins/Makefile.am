include $(top_srcdir)/config/utility.am

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

transactionpluginsdir = $(SIPX_LIBDIR)/transactionplugins
transactionplugins_LTLIBRARIES = \
    libGatewayDestPlugin.la

libGatewayDestPlugin_la_SOURCES = \
	GatewayDestPlugin.h \
	GatewayDestPlugin.cpp

libGatewayDestPlugin_la_CXXFLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include \
    -DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

libGatewayDestPlugin_la_LDFLAGS = \
    -module \
    -version-info ${version_Current}:${version_Revision}:${version_Age}

libGatewayDestPlugin_la_LIBADD = \
	-lboost_system-mt \
	@SIPXPORT_LIBS@ \
	@SIPXCOMMSERVER_LIBS@
