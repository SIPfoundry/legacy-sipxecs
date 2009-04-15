
lib_LTLIBRARIES = libsipXcommserverTest.la

libsipXcommserverTest_la_LDFLAGS = -static # no version info for static libs

INCLUDES = \
	-I$(top_srcdir)/include
	-I$(top_builddir)

libsipXcommserverTest_la_SOURCES =  \
	FileTestContext.cpp \
	SipDbTestContext.cpp \
	RegistrationDbTestContext.cpp

libsipXcommserverTest_la_LIBADD = \
	@SIPXUNIT_LIBS@

DISTCLEANFILES = Makefile.in
