## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am


INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = watchdog

watchdog_CXXFLAGS = \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\"

watchdog_SOURCES =  \
	$(top_builddir)/config/sipxpbx-buildstamp.h \
	$(top_builddir)/config/sipxpbx-buildstamp.cpp \
   EmailReporter.cpp \
   FailureReporterBase.cpp \
   WatchDogMain.cpp \
   MonitoredProcess.cpp \
	ProcMgmtRpc.cpp \
	DependencyList.cpp \
	processXMLCommon.cpp \
   WatchDog.cpp

watchdog_LDADD = \
    @SIPXTACK_LIBS@

noinst_HEADERS = \
	EmailReporter.h \
	FailureReporterBase.h \
	MonitoredProcess.h \
	ProcMgmtRpc.h \
	DependencyList.h \
	processXMLCommon.h \
	WatchDog.h

EXTRA_DIST = watchdog.sh.in

install-exec-hook :
	$(LocalizeSipXconfig) $(srcdir)/watchdog.sh.in > watchdog.sh
	$(INSTALL) -D -m 755 watchdog.sh $(DESTDIR)$(bindir)/watchdog.sh
	rm -f @SIPX_TMPDIR@/processAlias.dat
	rm -f @SIPX_TMPDIR@/locked.lck
	rm -rf @wwwdir@/cgi-bin/processmonitor

