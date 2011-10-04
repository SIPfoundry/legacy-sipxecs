include $(top_srcdir)/config/utility.am

lib_LTLIBRARIES = libsipxexample.la

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current = 1

# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision = 0

# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age = 0

INCLUDES = \
	-I$(srcdir)/../include

libsipxexample_la_LDFLAGS = \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libsipxexample_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libsipxexample_la_SOURCES = \
	ExampleLib.cpp

TESTS = $(check_PROGRAMS)

check_PROGRAMS = \
	test1

test1_LDADD = \
	@SIPXUNIT_LIBS@ \
	libsipxexample.la

test1_SOURCES = \
	ExampleLibTest.cpp
