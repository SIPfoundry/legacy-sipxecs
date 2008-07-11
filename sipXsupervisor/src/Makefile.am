## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

SUBDIRS = test

INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = sipxsupervisor

sipxsupervisor_SOURCES =  \
   sipXsupervisor.cpp

sipxsupervisor_CXXFLAGS =

sipxsupervisor_LDADD = \
	libsipXsupervisor.la

noinst_LTLIBRARIES = libsipXsupervisor.la

libsipXsupervisor_la_SOURCES =  \
	$(top_builddir)/config/sipxsupervisor-buildstamp.h \
	$(top_builddir)/config/sipxsupervisor-buildstamp.cpp \
   EmailReporter.cpp \
   FailureReporterBase.cpp \
   MonitoredProcess.cpp \
	ProcMgmtRpc.cpp \
	ImdbRpc.cpp \
	FileRpc.cpp \
	DependencyList.cpp \
	processXMLCommon.cpp \
	ProcessManager.cpp \
	ProcessResourceManager.cpp \
	ProcessResource.cpp \
	ImdbResourceManager.cpp \
	ImdbResource.cpp \
	SqldbResourceManager.cpp \
	SqldbResource.cpp \
	Process.cpp \
	ProcessCmd.cpp \
	SipxResource.cpp \
	FileResource.cpp \
	FileResourceManager.cpp \
   WatchDog.cpp

libsipXsupervisor_la_LIBADD = \
	 @SIPXCOMMSERVER_LIBS@ \
    @SIPXTACK_LIBS@

noinst_HEADERS = \
	EmailReporter.h \
	FailureReporterBase.h \
	MonitoredProcess.h \
	ProcMgmtRpc.h \
	ImdbRpc.h \
	FileRpc.h \
	DependencyList.h \
	processXMLCommon.h \
	ProcessManager.h \
	ProcessCmd.h \
	ProcessTask.h \
	Process.h \
	SipxResource.h \
	ProcessResource.h \
	ProcessResourceManager.h \
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
