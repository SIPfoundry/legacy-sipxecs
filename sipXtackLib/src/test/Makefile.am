## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite sandbox

INCLUDES = -I$(top_srcdir)/include

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
    net/NameValuePairTest.cpp \
    net/NameValuePairInsensitiveTest.cpp \
    net/NetAttributeTokenizerTest.cpp \
    net/NetBase64CodecTest.cpp \
    net/NetMd5CodecTest.cpp \
    net/UrlTest.cpp \
    net/BranchIdTest.cpp \
    net/CallIdTest.cpp \
    net/HttpBodyTest.cpp \
    net/HttpMessageTest.cpp \
    net/SipMessageTest.cpp \
    net/GetDigestAuthorizationDataTest.cpp \
    net/SignedUrlTest.cpp \
    net/HttpServerTest.cpp \
    net/SdpBodyTest.cpp \
    net/SipContactDbTest.cpp \
    net/SipDialogEventTest.cpp \
    net/SipDialogMonitorTest.cpp \
    net/SipDialogTest.cpp \
    net/SipLineTest.cpp \
    net/SipOutputProcessorTest.cpp \
    net/SipPresenceEventTest.cpp \
    net/SipProxyMessageTest.cpp \
    net/SipPublishContentMgrTest.cpp \
    net/SipRefreshManagerTest.cpp \
    net/SipServerShutdownTest.cpp \
    net/SipSrvLookupTest.cpp \
    net/SipSubscribeClientTest.cpp \
    net/SipSubscribeServerTest.cpp \
    net/SipSubscribeTestSupport.cpp \
    net/SipSubscriptionMgrTest.cpp \
    net/SipUserAgentTest.cpp \
    net/SipXauthIdentityTest.cpp \
    net/SipxNatRouteTest.cpp \
    net/XmlRpcTest.cpp

$(srcdir)/net/SipXauthIdentityTest.cpp: net/SipXauthIdentityTest.cpp.in
	$(srcdir)/net/refresh-hashes <$(srcdir)/net/SipXauthIdentityTest.cpp.in >$(srcdir)/net/SipXauthIdentityTest.cpp

EXTRA_DIST = \
    net/SipSrvLookupTest.named.zone \
    net/SipXauthIdentityTest.cpp.in \
    net/refresh-hashes

# [DEBUG ASSISTANT] List a single unit test here to isolate it for
# debugging or when developing a new unittest.  The executable
# 'sandbox' will then execute that one test.

# As a courtesy, please remove all unit tests from sandbox_SOURCES
# before checking checking in this file, although forgetting to has no
# ill effects.

sandbox_LDADD = $(testsuite_LDADD)
sandbox_CXXFLAGS = $(testsuite_CXXFLAGS)
sandbox_SOURCES =
