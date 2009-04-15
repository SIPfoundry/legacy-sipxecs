## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include 

noinst_PROGRAMS = musicserver

musicserver_LDADD = $(top_builddir)/src/libsipXcall.la

musicserver_SOURCES =  \
	main.cpp \
	listener.cpp \
	ActiveCall.cpp \
	CallObject.cpp

noinst_HEADERS = \
	listener.h \
	ActiveCall.h \
	CallObject.h

EXTRA_DIST = \
	default.wav

