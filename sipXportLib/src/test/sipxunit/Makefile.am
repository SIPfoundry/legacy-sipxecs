
INCLUDES = \
	-I$(top_srcdir)/src/test \
	-I$(top_srcdir)/include

lib_LTLIBRARIES = libsipXunit.la

# no version info for static libs
libsipXunit_la_LDFLAGS = \
	-static

libsipXunit_la_CXXFLAGS = \
	@CPPUNIT_CFLAGS@

libsipXunit_la_SOURCES =  \
    unitmain.cpp \
    TestUtilities.cpp \
    TestRunner.cpp \
    TestRunner.h \
    TestMonitor.cpp \
    TestMonitor.h \
    TestOutputter.cpp \
    TestOutputter.h \
	 TestOsSysLogListener.cpp \
	 TestOsSysLogListener.h \
    TestTemplate.cpp \
    FileTestContext.cpp

libsipXunit_la_LIBADD = \
	-lcppunit \
	-lstdc++

sipxincludedir = $(includedir)/sipxunit
sipxinclude_HEADERS = \
    TestUtilities.h \
    FileTestContext.h

DISTCLEANFILES = Makefile.in
