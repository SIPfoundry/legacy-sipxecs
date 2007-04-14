SUBDIRS = 

cgidir=@wwwdir@/cgi-bin
cbadmissioncgidir = $(cgidir)/cbadmission

cbadmissioncgi_PROGRAMS = cbadmission.cgi

cbadmission_cgi_LDADD = \
	@SIPXCOMMSERVER_LIBS@ \
   ../sipxcgi/libsipxcgi.la
   
cbadmission_cgi_CXXFLAGS = \
	-I$(top_srcdir)/include/ \
	-I$(top_srcdir)/src \
	@CGICC_CFLAGS@ \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

cbadmission_cgi_SOURCES = \
	main.cpp \
	ConferenceManager.cpp \
	ConferenceBridgeCGI.cpp \
	LoginCGI.cpp \
	TransferCGI.cpp \
	$(top_builddir)/config/sipxpbx-buildstamp.h \
	$(top_builddir)/config/sipxpbx-buildstamp.cpp

noinst_HEADERS = \
	ConferenceManager.h \
	ConferenceBridgeCGI.h \
	LoginCGI.h \
	TransferCGI.h

