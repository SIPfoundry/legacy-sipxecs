include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	sqaclient.h

lib_LTLIBRARIES = libsqaclient.la
INCLUDES = $(top_srcdir)/include

noinst_DATA = javac-sipxsqaclient

jardir = @SIPX_JAVADIR@/sipXcommons
jar_DATA = sipxsqaclient.jar

sipxsqaclient.jar : javac-sipxsqaclient
	jar cf $@ $(call JarInclude,$(JAVAROOT),.)

javac-sipxsqaclient : sipxsqaclient-src

sipxsqaclient_SRC = $(shell find . -name '*.java')

sipxsqaclient-src : sqaclient.h
	$(SWIG) -c++ -java -package org.sipfoundry.sqaclient -outcurrentdir $<

libsqaclient_la_LIBADD = \
	-lboost_date_time-mt \
	-lboost_filesystem-mt \
	-lboost_system-mt \
	-lboost_thread-mt \
	-lboost_program_options-mt \
	-lpthread \
	-lzmq

sqaclient_wrap.cxx : sipxsqaclient-src

libsqaclient_la_SOURCES = \
	sqaclient_wrap.cxx

CLEANFILES = *.java $(libsqaclient_la_SOURCES)
