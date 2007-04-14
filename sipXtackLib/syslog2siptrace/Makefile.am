## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include 

bin_PROGRAMS = syslog2siptrace

syslog2siptrace_LDADD = $(top_builddir)/src/libsipXtack.la

syslog2siptrace_SOURCES = main.cpp

bin_SCRIPTS = merge-logs siptrace-merge

EXTRA_DIST = \
    merge-logs \
    siptrace-merge
