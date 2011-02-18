## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include

noinst_PROGRAMS = basicImClient

basicImClient_LDADD = $(top_builddir)/src/libsipXtack.la -lboost_system-mt -lboost_thread-mt

basicImClient_SOURCES =  basicImClient.cpp
