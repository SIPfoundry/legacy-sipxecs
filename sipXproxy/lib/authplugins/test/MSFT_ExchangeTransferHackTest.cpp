//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/TestCase.h"
#include "sipxunit/TestUtilities.h"
#include "utl/PluginHooks.h"
#include "os/OsConfigDb.h"

#include "net/SipMessage.h"
#include "net/SipUserAgent.h"

#include "ForwardRules.h"
#include "SipRouter.h"

#include "testlib/FileTestContext.h"
#include "MSFT_ExchangeTransferHack.h"

class MSFT_ExchangeTransferHackTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(MSFT_ExchangeTransferHackTest);

   CPPUNIT_TEST(testConstructor);

   CPPUNIT_TEST(nonInvite);
   CPPUNIT_TEST(normalInvite);
   CPPUNIT_TEST(InviteWithReplaces);
   CPPUNIT_TEST(ReferWithReplaces);
   CPPUNIT_TEST(UnAuthenticatedRefer);
   CPPUNIT_TEST(AuthenticatedRefer);
   CPPUNIT_TEST(UnAuthenticatedForiegnRefer);
   CPPUNIT_TEST(BadReferNotExchange);
   CPPUNIT_TEST(BadReferFromExchange);
   CPPUNIT_TEST(BadReferFromExchangeWithPort);
   
   CPPUNIT_TEST_SUITE_END();

public:

   static FileTestContext* TransferTestContext;
   static MSFT_ExchangeTransferHack* xferctl;
   static SipUserAgent     testUserAgent;
   static SipRouter*       testSipRouter;
   
   void setUp()
      {
         TransferTestContext = new FileTestContext(TEST_DATA_DIR "/transfer-control",
                                                   TEST_WORK_DIR "/msft-xfer-hack-context");
         TransferTestContext->inputFile("domain-config");
         TransferTestContext->setSipxDir(SipXecsService::ConfigurationDirType);
      }

   void tearDown()
      {
      }

   void testConstructor()
      {
         /*
          * This test exists to initialize the singleton plugin.
          * Doing it as a static ran into ordering problems.
          */
         CPPUNIT_ASSERT((xferctl=dynamic_cast<MSFT_ExchangeTransferHack*>(getAuthPlugin("msft"))));

         OsConfigDb xferConfigDb;
         xferConfigDb.set(MSFT_ExchangeTransferHack::RecognizerConfigKey, "^RTCC/");
         xferctl->readConfig(xferConfigDb);

         testUserAgent.setIsUserAgent(FALSE);

         testUserAgent.setUserAgentHeaderProperty("sipXecs/testproxy");

         testUserAgent.setForking(FALSE);  // Disable forking

         OsConfigDb configDb;
         configDb.set("SIPX_PROXY_AUTHENTICATE_ALGORITHM", "MD5");
         configDb.set("SIPX_PROXY_HOSTPORT", "sipx.example.edu");

         testSipRouter = new SipRouter(testUserAgent, mForwardingRules, configDb);
         xferctl->announceAssociatedSipRouter( testSipRouter );
      }

   // Test that an in-dialog request without a replaces header is not affected
   void nonInvite()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:someone@somewhere");

         const char* message =
            "INFO sip:someone@somewhere SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:someone@somewhere;tag=totag\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 5 INFO\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         UtlString myRouteName("myhost.example.com");
         RouteState routeState( testMsg, noRemovedRoutes, myRouteName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         UtlString method("INFO");
         bool bSpiralingRequest = false;
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         
         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == xferctl->authorizeAndModify(identity,
                                                       requestUri,
                                                       routeState,
                                                       method,
                                                       priorResult,
                                                       testMsg,
                                                       bSpiralingRequest,
                                                       rejectReason
                                                       ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         // check that the message has not been modified
         UtlString outputMsg;
         ssize_t    outputSize;
         testMsg.getBytes(&outputMsg, &outputSize);

         ASSERT_STR_EQUAL(message, outputMsg.data());
      }

   // Test that a dialog-forming INVITE without a replaces header is not affected
   void normalInvite()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:someone@somewhere");

         const char* message =
            "INVITE sip:someone@somewhere SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:someone@somewhere\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         UtlString myRouteName("myhost.example.com");
         RouteState routeState( testMsg, noRemovedRoutes, myRouteName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         UtlString method("INVITE");
         bool bSpiralingRequest = false;
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         
         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == xferctl->authorizeAndModify(identity,
                                                       requestUri,
                                                       routeState,
                                                       method,
                                                       priorResult,
                                                       testMsg,
                                                       bSpiralingRequest,
                                                       rejectReason
                                                       ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         // check that the message has not been modified
         UtlString outputMsg;
         ssize_t    outputSize;
         testMsg.getBytes(&outputMsg, &outputSize);

         ASSERT_STR_EQUAL(message, outputMsg.data());
      }



   // Test that an out-of-dialog INVITE with a Replaces header is allowed
   void InviteWithReplaces()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:someone@somewhere");

         const char* message =
            "INVITE sip:someone@somewhere SIP/2.0\r\n"
            "Replaces: valid@callid;to-tag=totagvalue;from-tag=fromtagvalue\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:someone@somewhere\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         UtlString myRouteName("myhost.example.com");
         RouteState routeState( testMsg, noRemovedRoutes, myRouteName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         UtlString method("INVITE");
         bool bSpiralingRequest = false;
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         
         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == xferctl->authorizeAndModify(identity,
                                                       requestUri,
                                                       routeState,
                                                       method,
                                                       priorResult,
                                                       testMsg,
                                                       bSpiralingRequest,
                                                       rejectReason
                                                       ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         // check that the message has not been modified
         UtlString outputMsg;
         ssize_t    outputSize;
         testMsg.getBytes(&outputMsg, &outputSize);

         ASSERT_STR_EQUAL(message, outputMsg.data());
      }

   // Test that a REFER with Replaces is allowed
   void ReferWithReplaces()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:someone@somewhere");

         const char* message =
            "REFER sip:someone@somewhere SIP/2.0\r\n"
            "Refer-To: other@elsewhere?replaces=valid@callid;to-tag=totagvalue;from-tag=fromtagvalue\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:someone@somewhere\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         UtlString myRouteName("myhost.example.com");
         RouteState routeState( testMsg, noRemovedRoutes, myRouteName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         UtlString method("REFER");
         bool bSpiralingRequest = false;
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         
         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == xferctl->authorizeAndModify(identity,
                                                       requestUri,
                                                       routeState,
                                                       method,
                                                       priorResult,
                                                       testMsg,
                                                       bSpiralingRequest,
                                                       rejectReason
                                                       ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         // check that the message has not been modified
         UtlString outputMsg;
         ssize_t    outputSize;
         testMsg.getBytes(&outputMsg, &outputSize);

         ASSERT_STR_EQUAL(message, outputMsg.data());
      }


   // Test that an unauthenticated REFER without Replaces and a good target is not modified
   void UnAuthenticatedRefer()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:someone@somewhere");

         const char* message =
            "REFER sip:someone@somewhere SIP/2.0\r\n"
            "Refer-To: other@example.edu\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:someone@somewhere\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         UtlString myRouteName("myhost.example.com");
         RouteState routeState( testMsg, noRemovedRoutes, myRouteName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         UtlString method("REFER");
         bool bSpiralingRequest = false;
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         
         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == xferctl->authorizeAndModify(identity,
                                                       requestUri,
                                                       routeState,
                                                       method,
                                                       priorResult,
                                                       testMsg,
                                                       bSpiralingRequest,
                                                       rejectReason
                                                       ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         // check that the message has not been modified
         UtlString outputMsg;
         ssize_t    outputSize;
         testMsg.getBytes(&outputMsg, &outputSize);

         ASSERT_STR_EQUAL(message, outputMsg.data());
      }

   
   // Test that an authenticated REFER without Replaces to a good target is allowed and not modified
   void AuthenticatedRefer()
      {
         UtlString identity("controller@domain"); // an authenticated identity
         Url requestUri("sip:someone@somewhere");

         const char* message =
            "REFER sip:someone@somewhere SIP/2.0\r\n"
            "Refer-To: other@example.edu\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:someone@somewhere\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         UtlString myRouteName("myhost.example.com");
         RouteState routeState( testMsg, noRemovedRoutes, myRouteName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         UtlString method("REFER");
         bool bSpiralingRequest = false;
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         
         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == xferctl->authorizeAndModify(identity,
                                                       requestUri,
                                                       routeState,
                                                       method,
                                                       priorResult,
                                                       testMsg,
                                                       bSpiralingRequest,
                                                       rejectReason
                                                       ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         UtlString modifiedReferToStr;
         CPPUNIT_ASSERT(testMsg.getReferToField(modifiedReferToStr));

         Url modifiedReferTo(modifiedReferToStr);
         CPPUNIT_ASSERT(Url::SipUrlScheme == modifiedReferTo.getScheme());

         // check that the message has not been modified
         UtlString outputMsg;
         ssize_t    outputSize;
         testMsg.getBytes(&outputMsg, &outputSize);

         ASSERT_STR_EQUAL(message, outputMsg.data());
      }

   // Test that an unauthenticated REFER without Replaces to a foreign target is not modified
   void UnAuthenticatedForiegnRefer()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:someone@somewhere");

         const char* message =
            "REFER sip:someone@somewhere SIP/2.0\r\n"
            "Refer-To: other@elsewhere.edu\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:someone@somewhere\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         UtlString myRouteName("myhost.example.com");
         RouteState routeState( testMsg, noRemovedRoutes, myRouteName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         UtlString method("REFER");
         bool bSpiralingRequest = false;
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         
         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == xferctl->authorizeAndModify(identity,
                                                       requestUri,
                                                       routeState,
                                                       method,
                                                       priorResult,
                                                       testMsg,
                                                       bSpiralingRequest,
                                                       rejectReason
                                                       ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         // check that the message has not been modified
         UtlString outputMsg;
         ssize_t    outputSize;
         testMsg.getBytes(&outputMsg, &outputSize);

         ASSERT_STR_EQUAL(message, outputMsg.data());
      }
   
   // Test that a buggy REFER without Replaces not from Exchange is not modified
   void BadReferNotExchange()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:someone@foreign.example.edu");

         const char* message =
            "REFER sip:someone@foreign.example.edu SIP/2.0\r\n"
            "Refer-To: other@foreign.example.edu\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:someone@somewhere\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: OtherBuggy/2\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         UtlString myRouteName("myhost.example.com");
         RouteState routeState( testMsg, noRemovedRoutes, myRouteName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         UtlString method("REFER");
         bool bSpiralingRequest = false;
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         
         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == xferctl->authorizeAndModify(identity,
                                                       requestUri,
                                                       routeState,
                                                       method,
                                                       priorResult,
                                                       testMsg,
                                                       bSpiralingRequest,
                                                       rejectReason
                                                       ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         // check that the message has not been modified
         UtlString outputMsg;
         ssize_t    outputSize;
         testMsg.getBytes(&outputMsg, &outputSize);

         ASSERT_STR_EQUAL(message, outputMsg.data());
      }
   
   // Test that a buggy REFER without Replaces from Exchange is modified
   void BadReferFromExchange()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:someone@foreign.example.edu");

         const char* message =
            "REFER sip:someone@foreign.example.edu SIP/2.0\r\n"
            "Refer-To: other@foreign.example.edu\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:someone@somewhere\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: RTCC/2\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         UtlString myRouteName("myhost.example.com");
         RouteState routeState( testMsg, noRemovedRoutes, myRouteName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         UtlString method("REFER");
         bool bSpiralingRequest = false;
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         
         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == xferctl->authorizeAndModify(identity,
                                                       requestUri,
                                                       routeState,
                                                       method,
                                                       priorResult,
                                                       testMsg,
                                                       bSpiralingRequest,
                                                       rejectReason
                                                       ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         // check that the target has been modified to our domain
         UtlString modifiedReferToStr;
         CPPUNIT_ASSERT(testMsg.getReferToField(modifiedReferToStr));

         ASSERT_STR_EQUAL("sip:other@example.edu", modifiedReferToStr.data());
      }

   // Test that a buggy REFER without Replaces from Exchange is modified
   void BadReferFromExchangeWithPort()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:someone@10.1.1.5:56777");

         const char* message =
            "REFER sip:someone@10.1.1.5:56777 SIP/2.0\r\n"
            "Refer-To: other@10.1.1.5:56777\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:someone@somewhere\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: RTCC/2\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         UtlString myRouteName("myhost.example.com");
         RouteState routeState( testMsg, noRemovedRoutes, myRouteName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         UtlString method("REFER");
         bool bSpiralingRequest = false;
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         
         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == xferctl->authorizeAndModify(identity,
                                                       requestUri,
                                                       routeState,
                                                       method,
                                                       priorResult,
                                                       testMsg,
                                                       bSpiralingRequest,
                                                       rejectReason
                                                       ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         // check that the target has been modified to our domain
         UtlString modifiedReferToStr;
         CPPUNIT_ASSERT(testMsg.getReferToField(modifiedReferToStr));

         ASSERT_STR_EQUAL("sip:other@example.edu", modifiedReferToStr.data());
      }

private:
   ForwardRules  mForwardingRules;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MSFT_ExchangeTransferHackTest);

MSFT_ExchangeTransferHack* MSFT_ExchangeTransferHackTest::xferctl;
FileTestContext* MSFT_ExchangeTransferHackTest::TransferTestContext;
SipUserAgent     MSFT_ExchangeTransferHackTest::testUserAgent;
SipRouter*       MSFT_ExchangeTransferHackTest::testSipRouter;
