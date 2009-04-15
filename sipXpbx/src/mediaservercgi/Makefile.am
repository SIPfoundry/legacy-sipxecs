SUBDIRS = 

cgidir=@wwwdir@/cgi-bin
mediaservercgidir = $(cgidir)/voicemail

mediaservercgi_PROGRAMS = mediaserver.cgi

mediaserver_cgi_LDADD = \
   ../mailboxmgr/libmailboxmgr.la \
   ../sipxcgi/libsipxcgi.la \
	@GIPS_NEQ_OBJS@

mediaserver_cgi_CXXFLAGS = \
	-I$(top_srcdir)/include/ \
	-I$(top_srcdir)/src \
	@CGICC_CFLAGS@ \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\"

mediaserver_cgi_SOURCES = \
	main.cpp \
	$(top_builddir)/config/sipxpbx-buildstamp.h \
	$(top_builddir)/config/sipxpbx-buildstamp.cpp
