## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/config \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../sipXcallLib/include

bin_PROGRAMS = sipxacd

EXTRA_DIST = \
	$(AUDIO_DIST)

sipxacd_LDADD = \
    @SIPXCALL_LIBS@ \
    @SIPXMEDIAADAPTER_LIBS@ \
    @SIPXCOMMSERVER_LIBS@ \
    @GIPS_NEQ_OBJS@

sipxacd_CXXFLAGS = \
        -I$(top_builddir)/config \
        -I$(top_builddir) \
        -DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
        -DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
        -DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
        -DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"\
        -DSIPX_VARDIR=\"@SIPX_VARDIR@\"

sipxacd_SOURCES =  \
	main.cpp \
	ACDAgent.cpp \
	ACDAgentMsg.cpp \
	ACDAgentManager.cpp \
	ACDCall.cpp \
	ACDCallManager.cpp \
	ACDCallMsg.cpp \
	ACDCallRouteState.cpp \
	ACDLine.cpp \
	ACDLineManager.cpp \
	ACDQueue.cpp \
	ACDQueue_Circular.cpp \
	ACDQueue_Linear.cpp \
	ACDQueue_LongestIdle.cpp \
	ACDQueue_RingAll.cpp \
	ACDQueueManager.cpp \
	ACDQueueMsg.cpp \
	ACDAudio.cpp \
	ACDAudioManager.cpp \
	ACDServer.cpp \
	ACDRtRecord.cpp

noinst_HEADERS = \
	ACDAgent.h \
	ACDAgentMsg.h \
	ACDAgentManager.h \
	ACDCall.h \
	ACDCallManager.h \
	ACDCallMsg.h \
	ACDCallRouteState.h \
	ACDLine.h \
	ACDLineManager.h \
	ACDQueue.h \
	ACDQueue_Circular.h \
	ACDQueue_Linear.h \
	ACDQueue_LongestIdle.h \
	ACDQueue_RingAll.h \
	ACDQueueManager.h \
	ACDQueueMsg.h \
	ACDAudio.h \
	ACDAudioManager.h \
	ACDServer.h \
	ACDRtRecord.h \
        BusyTone.h \
	ConfirmationTone.h \
	RingbackTone.h \
	ConfirmShortTone.h

install-exec-hook :

install-data-hook :
