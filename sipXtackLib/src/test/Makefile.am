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
    net/XmlRpcTest.cpp \
    net/SipXlocationInfoTest.cpp

$(srcdir)/net/SipXauthIdentityTest.cpp: net/SipXauthIdentityTest.cpp.in
	$(srcdir)/net/refresh-hashes <$(srcdir)/net/SipXauthIdentityTest.cpp.in >$(srcdir)/net/SipXauthIdentityTest.cpp

EXTRA_DIST = \
    net/SipSrvLookupTest.named.zone \
    net/SipXauthIdentityTest.cpp.in \
    net/refresh-hashes
