## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sipxdialog

sipxdialog_LDADD = \
        @SIPXCOMMSERVER_LIBS@

sipxdialog_CXXFLAGS = \
        -I$(top_builddir)/config \
        -DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
        -DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
        -DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
        -DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipxdialog_SOURCES =  \
    ../../config/sipxpbx-buildstamp.h \
    ../../config/sipxpbx-buildstamp.cpp \
	main.cpp \
	dmConfig.cpp

noinst_HEADERS = \
	dmConfig.h

install-exec-hook :

install-data-hook :

