## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite

INCLUDES = -I$(top_srcdir)/include -I../

# Make TESTDIR an absolute path to the directory containing
# SipSrvLookupTest.named.*
TESTDIR=$(if $(patsubst /%,,$(top_srcdir)),$(shell pwd)/$(top_srcdir),$(top_srcdir))/src/test/net

# Only -D NAMED_PROGRAM if ./configure has found named and thus set
# @NAMED_PROGRAM@ to non-null.
testsuite_CXXFLAGS = \
    -DTESTDIR=\"$(TESTDIR)\" \
    $(if @NAMED_PROGRAM@,-DNAMED_PROGRAM=\"@NAMED_PROGRAM@\")

testsuite_LDADD = \
    @SIPXUNIT_LIBS@ \
    ../libsipXtack.la

testsuite_SOURCES = \
    net/UrlTest.cpp \
    net/SipMessageTest.cpp 

$(srcdir)/net/SipXauthIdentityTest.cpp: net/SipXauthIdentityTest.cpp.in
	$(srcdir)/net/refresh-hashes <$(srcdir)/net/SipXauthIdentityTest.cpp.in >$(srcdir)/net/SipXauthIdentityTest.cpp

EXTRA_DIST = \
    net/SipSrvLookupTest.named.zone \
    net/SipXauthIdentityTest.cpp.in \
    net/refresh-hashes
