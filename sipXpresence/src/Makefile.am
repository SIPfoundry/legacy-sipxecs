## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sipxpresence

sipxpresence_LDADD = \
        @SIPXCALL_LIBS@ \
        @SIPXMEDIAADAPTER_LIBS@ \
        @GIPS_NEQ_OBJS@ \
	@SIPXCOMMSERVER_LIBS@

sipxpresence_CXXFLAGS = \
        -DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
        -DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
        -DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
        -DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

sipxpresence_SOURCES =  \
	main.cpp

noinst_HEADERS =

install-exec-hook :

install-data-hook :
	$(INSTALL) -d -m 755 $(DESTDIR)@SIPX_VARDIR@/sipxpresence
