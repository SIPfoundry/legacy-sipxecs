## Process this file with automake to produce Makefile.in

include $(top_srcdir)/config/subdir.am

INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/config


bin_PROGRAMS = xsdvalid

xsdvalid_LDADD = \
	@SIPXPORT_LIBS@ \
	@XERCES_LIBS@

xsdvalid_SOURCES = \
	 ../../config/sipxcommserverlib-buildstamp.h \
	 ../../config/sipxcommserverlib-buildstamp.cpp \
	xsdvalid.cpp
