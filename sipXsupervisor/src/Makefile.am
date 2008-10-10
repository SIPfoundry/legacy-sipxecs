## SipxProcess this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

SUBDIRS = test

INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = sipxsupervisor

sipxsupervisor_SOURCES =  \
   sipXsupervisor.cpp

sipxsupervisor_CXXFLAGS =

sipxsupervisor_LDADD = \
	libsipXsupervisor.la \
   libsipXalarms.la

noinst_LTLIBRARIES = libsipXsupervisor.la libsipXalarms.la

libsipXsupervisor_la_SOURCES =  \
	$(top_builddir)/config/sipxsupervisor-buildstamp.h \
	$(top_builddir)/config/sipxsupervisor-buildstamp.cpp \
	sipXsupervisor.cpp \
	ProcMgmtRpc.cpp \
	AlarmRpc.cpp \
	ImdbRpc.cpp \
	FileRpc.cpp \
	DependencyList.cpp \
	SipxProcessManager.cpp \
	SipxProcessResourceManager.cpp \
	SipxProcessResource.cpp \
	ImdbResourceManager.cpp \
	ImdbResource.cpp \
	SqldbResourceManager.cpp \
	SqldbResource.cpp \
	SipxProcess.cpp \
	SipxProcessFsm.cpp \
	SipxProcessCmd.cpp \
	SipxResource.cpp \
	FileResource.cpp \
	FileResourceManager.cpp \
   WatchDog.cpp

noinst_HEADERS = \
	ProcMgmtRpc.h \
	AlarmRpc.h \
	ImdbRpc.h \
	FileRpc.h \
	DependencyList.h \
	SipxProcessManager.h \
	SipxProcessCmd.h \
	SipxProcess.h \
	SipxProcessFsm.h \
	SipxResource.h \
	SipxProcessResource.h \
	SipxProcessResourceManager.h \
	FileResource.h \
	FileResourceManager.h \
	ImdbResource.h \
	ImdbResourceManager.h \
	SqldbResource.h \
	SqldbResourceManager.h \
	WatchDog.h
	ImdbResource.h \
	SqldbResource.h \
	WatchDog.h

libsipXsupervisor_la_LIBADD = \
	 @SIPXCOMMSERVER_LIBS@ \
    @SIPXTACK_LIBS@

libsipXalarms_la_CXXFLAGS = $(sipxsupervisor_CXXFLAGS)

libsipXalarms_la_LIBADD = \
	 @SIPXCOMMSERVER_LIBS@ \
    @SIPXTACK_LIBS@

libsipXalarms_la_SOURCES = \
	AlarmUtils.cpp \
	EmailNotifier.cpp \
	EmailSendTask.cpp \
	LogNotifier.cpp \
	NotifierBase.cpp \
	AlarmData.cpp \
	AlarmServer.cpp \
	AlarmUtils.h \
	EmailNotifier.h \
	EmailSendTask.h \
	LogNotifier.h \
	NotifierBase.h \
	AlarmData.h \
	AlarmServer.h
