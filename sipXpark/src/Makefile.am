## Process this file with automake to produce Makefile.in

bin_PROGRAMS = sipxpark

audio_path=@SIPX_VARDIR@/parkserver/music

AUDIO_DIST = \
	audio/default.wav

EXTRA_DIST = \
	$(AUDIO_DIST)

sipxpark_LDADD = \
	@SIPXCALL_LIBS@ \
	@SIPXMEDIAADAPTER_LIBS@ \
	@SIPXCOMMSERVER_LIBS@ \
	@GIPS_NEQ_OBJS@

sipxpark_CXXFLAGS = \
        -I$(top_builddir)/config \
        -I$(top_builddir) \
        -DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
        -DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
        -DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
        -DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"\
        -DSIPX_VARDIR=\"@SIPX_VARDIR@\"

sipxpark_SOURCES =  \
	main.cpp \
	OrbitListener.cpp \
	ParkedCallObject.cpp

noinst_HEADERS = \
	OrbitListener.h \
	ParkedCallObject.h

install-exec-hook :

install-data-hook :
	@for f in $(AUDIO_DIST); \
	do \
	   target=`basename $$f` ; \
	   if [ -f $(DESTDIR)$(audio_path)/$$target ]; \
	   then \
	      echo "Using existing $(audio_path)/$$target"; \
	   else \
	      echo ""; echo "Installing default $(audio_path)/$$target"; \
	      $(INSTALL) -D -m 644 $(srcdir)/$$f $(DESTDIR)$(audio_path)/$$target; \
	   fi; \
	done
