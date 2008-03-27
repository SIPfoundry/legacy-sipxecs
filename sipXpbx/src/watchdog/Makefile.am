## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am


INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = watchdog

watchdog_CXXFLAGS = \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPXPBXUSER=\"@SIPXPBXUSER@\" \
	-DSIPXPBXGROUP=\"@SIPXPBXGROUP@\"

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
	 @SIPXCOMMSERVER_LIBS@ \
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

# These commands parallel the post-install commands in sipxpbx.spec.in.
# But: Do not "rm -rf @wwwdir@/cgi-bin/processmonitor" here, because if
# that directory was installed by an RPM, a non-root builder may not
# be able to remove it.  That directory's presence is harmless clutter.
install-exec-hook :
	$(LocalizeSipXconfig) $(srcdir)/watchdog.sh.in > watchdog.sh
	$(INSTALL) -D -m 755 watchdog.sh $(DESTDIR)$(bindir)/watchdog.sh
	rm -f @SIPX_TMPDIR@/processAlias.dat
	rm -f @SIPX_TMPDIR@/locked.lck
