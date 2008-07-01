## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = sipxsupervisor

sipxsupervisor_CXXFLAGS = \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPXPBXUSER=\"@SIPXPBXUSER@\" \
	-DSIPXPBXGROUP=\"@SIPXPBXGROUP@\"

sipxsupervisor_SOURCES =  \
	$(top_builddir)/config/sipxsupervisor-buildstamp.h \
	$(top_builddir)/config/sipxsupervisor-buildstamp.cpp \
   EmailReporter.cpp \
   FailureReporterBase.cpp \
   sipXsupervisor.cpp \
   MonitoredProcess.cpp \
	ProcMgmtRpc.cpp \
	ImdbRpc.cpp \
	FileRpc.cpp \
	DependencyList.cpp \
	processXMLCommon.cpp \
   WatchDog.cpp

sipxsupervisor_LDADD = \
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
	Process.h \
	SipxResource.h \
	ProcessResource.h \
	FileResource.h \
	ImdbResource.h \
	SqldbResource.h \
	WatchDog.h
