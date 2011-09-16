bin_PROGRAMS = sipxexample

sipxexample_CXXFLAGS = \
	-I$(srdir)

sipxexample_LDADD = \
	@SIPXCOMMSERVER_LIBS@

noinst_HEADERS = \
	ExampleApp.h

sipxexample_SOURCES =  \
	ExampleMain.cpp \
	ExampleApp.cpp

TESTS = $(check_PROGRAMS)

check_PROGRAMS = \
	test1

test1_LDADD = \
	@SIPXUNIT_LIBS@ \
	$(sipxexample_LDADD)

test1_SOURCES = \
	ExampleAppTest.cpp \
	ExampleApp.cpp
