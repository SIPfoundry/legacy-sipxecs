## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sipxrls sipxsub

SUBDIRS = . test

EXTRA_DIST =

sipxrls_LDADD = \
        @SIPXCALL_LIBS@ \
	@SIPXMEDIAADAPTER_LIBS@ \
	@SIPXCOMMSERVER_LIBS@ \
	@GIPS_NEQ_OBJS@ \
	-lboost_thread-mt \
	-lmongoclient \
	-lzmq

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

sipxsub_LDADD = \
	-lboost_date_time-mt \
	-lboost_filesystem-mt \
	-lboost_system-mt \
	-lboost_thread-mt \
	-lboost_program_options-mt \
	-lpthread \
	-ldum \
	-lresip \
	-lrutil \
	-lrt \
	-ldl \
	-lssl \
	-lcrypto \
        -lmongoclient \
	@SIPXPORT_LIBS@ \
	@SIPXCOMMSERVER_LIBS@

sipxsub_CXXFLAGS = \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-I$(top_srcdir)/include

sipxsub_SOURCES =  sipxsub.cpp \
    ResipLogger.h \
    ResipLogger.cpp \
    SubscriptionDialog.h \
    SubscriptionDialog.cpp \
    EventSubscriber.h \
    EventSubscriber.cpp

install-exec-hook :
