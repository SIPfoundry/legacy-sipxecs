## Process this file with automake to produce Makefile.in

SUBDIRS = test

bin_PROGRAMS = sipregistrar

sipregistrar_CXXFLAGS = \
	-I$(top_srcdir)/include \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipregistrar_SOURCES =  \
    main.cpp

sipregistrar_LDADD = \
	libsipXregistry.la \
	@GIPS_NEQ_OBJS@

sipregistrar_LDFLAGS = \
	-rdynamic

noinst_LTLIBRARIES = libsipXregistry.la

libsipXregistry_la_CXXFLAGS = $(sipregistrar_CXXFLAGS)

libsipXregistry_la_LIBADD = \
	-rdynamic

libsipXregistry_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@ \
        -lmongoclient \
        -lboost_system-mt

libsipXregistry_la_SOURCES = \
    RedirectPlugin.cpp \
    RedirectResumeMsg.cpp \
    RedirectResumeMsg.h \
    RedirectSuspend.cpp \
    RegisterEventServer.cpp \
    RegisterEventServer.h \
    SipRedirectServer.cpp \
    SipRegistrar.cpp \
    SipRegistrarServer.cpp \
    SipRegistrarServer.h

