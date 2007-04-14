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
    WatchDog.cpp

watchdog_LDADD = \
    ../processcgi/libprocesscgi.la

noinst_HEADERS = \
	EmailReporter.h \
	FailureReporterBase.h \
	MonitoredProcess.h \
	WatchDog.h

EXTRA_DIST = watchdog.sh.in

install-exec-hook :
	$(LocalizeSipXconfig) $(srcdir)/watchdog.sh.in > watchdog.sh
	$(INSTALL) -D -m 755 watchdog.sh $(DESTDIR)$(bindir)/watchdog.sh
