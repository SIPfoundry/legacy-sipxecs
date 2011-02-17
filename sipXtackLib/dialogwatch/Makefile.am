## Process this file with automake to produce Makefile.in

EXTRA_DIST = \
    dialogdisplay

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_builddir)/include

bin_PROGRAMS = dialogwatch

bin_SCRIPTS = dialogdisplay

# Program that monitors dialog events.
dialogwatch_CXXFLAGS =
dialogwatch_LDADD = $(top_builddir)/src/libsipXtack.la -lboost_system-mt -lboost_thread-mt
dialogwatch_SOURCES = \
    dialogwatch.cpp
