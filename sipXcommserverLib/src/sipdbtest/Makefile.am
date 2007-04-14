## Process this file with automake to produce Makefile.in

include $(top_srcdir)/config/subdir.am

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/include/fastdb


bin_PROGRAMS = sipdbtest

sipdbtest_LDADD = \
	$(top_builddir)/src/libsipXcommserver.la

sipdbtest_CXXFLAGS = \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\" \
   -DSIPXCHANGE_USERNAME=\"@SIPXPBXUSER@\"

sipdbtest_SOURCES = \
	 ../../config/sipxcommserverlib-buildstamp.h \
	 ../../config/sipxcommserverlib-buildstamp.cpp \
    DisplayTask.cpp \
    HeartBeatMonitor.cpp \
    IMDBTaskMonitor.cpp \
    IMDBWorkerTask.cpp \
    ImportTask.cpp \
    KeepAliveTask.cpp \
    main.cpp \
    DisplayTask.h \
    HeartBeatMonitor.h \
    IMDBTaskMonitor.h \
    IMDBWorkerTask.h \
    ImportTask.h \
    KeepAliveTask.h

EXTRA_DIST = \
    sipdbtest.sh.in

install-exec-hook :
	$(LocalizeSipXconfig) $(srcdir)/sipdbtest.sh.in > sipdbtest.sh
	$(INSTALL) -D -m 755 sipdbtest.sh $(DESTDIR)$(bindir)/sipdbtest.sh
