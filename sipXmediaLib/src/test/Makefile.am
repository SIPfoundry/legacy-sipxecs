INCLUDES = -I$(top_srcdir)/include

TESTS = testsuite

check_PROGRAMS = testsuite sandbox

testsuite_LDADD = \
    @SIPXUNIT_LIBS@ \
    ../libsipXmedia.la \
    @GIPS_NEQ_OBJS@

# NOTE: Several media releated units tests are located in
## sipXcallLib because they made references to CallManager
testsuite_SOURCES = \
    mp/MpMediaTaskTest.cpp \
    mp/MpFlowGraphTest.cpp \
    mp/MpResourceTest.cpp \
    mp/MpTestResource.h \
    mp/MpTestResource.cpp


# [DEBUG ASSISTANT] List a single unittest to isoloate a unittest for
# debugging or when developing a new unittest. As courtesy remove all
# unittests before checking in, although forgetting to has no ill
# effects. To build and run:
#
#   cd src/test
#   make sandbox
#   ./sandbox
#
sandbox_LDFLAGS = $(testsuite_LDFLAGS)

sandbox_LDADD = $(testsuite_LDADD)

sandbox_SOURCES = \
    mp/MpMediaTaskTest.cpp
