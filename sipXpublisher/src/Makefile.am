## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include

bin_PROGRAMS = sipstatus

sipstatus_LDADD = \
	@SIPXCOMMSERVER_LIBS@

sipstatus_CXXFLAGS = \
	@SSL_CXXFLAGS@ \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipstatus_SOURCES =  \
    $(top_builddir)/config/sipxpublisher-buildstamp.h \
    $(top_builddir)/config/sipxpublisher-buildstamp.cpp \
    statusserver/main.cpp \
    statusserver/MwiPlugin.cpp \
    statusserver/Notifier.cpp \
    statusserver/PluginXmlParser.cpp \
    statusserver/StatusPluginReference.cpp \
    statusserver/StatusServer.cpp \
    statusserver/SubscribePersistThread.cpp \
    statusserver/SubscribeServerPluginBase.cpp \
    statusserver/SubscribeServerThread.cpp \
    statusserver/WebServer.cpp
