## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sipxrls

SUBDIRS = . test

EXTRA_DIST =

sipxrls_LDADD = \
        @SIPXCALL_LIBS@ \
	@SIPXMEDIAADAPTER_LIBS@ \
	@SIPXCOMMSERVER_LIBS@ \
	@GIPS_NEQ_OBJS@

sipxrls_CXXFLAGS = \
        -DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
        -DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
        -DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
        -DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipxrls_SOURCES =  \
	main.cpp \
	ContactSet.cpp \
	ResourceCache.cpp \
	ResourceCached.cpp \
	ResourceInstance.cpp \
	ResourceList.cpp \
	ResourceListFileReader.cpp \
	ResourceListMsg.cpp \
	ResourceListServer.cpp \
	ResourceListSet.cpp \
	ResourceListTask.cpp \
	ResourceNotifyReceiver.cpp \
	ResourceReference.cpp \
	ResourceSubscriptionReceiver.cpp \
	RlsSubscribePolicy.cpp \
	SubscriptionSet.cpp

noinst_HEADERS = \
	ContactSet.h \
	ResourceCache.h \
	ResourceCached.h \
	ResourceInstance.h \
	ResourceList.h \
	ResourceListFileReader.h \
	ResourceListMsg.h \
	ResourceListServer.h \
	ResourceListSet.h \
	ResourceListTask.h \
	ResourceNotifyReceiver.h \
	ResourceReference.h \
	ResourceSubscriptionReceiver.h \
	RlsSubscribePolicy.h \
	SubscriptionSet.h \
	main.h

install-exec-hook :
