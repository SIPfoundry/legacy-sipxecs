//
// Copyright (C) 2008 Nortel Networks., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/TestCase.h"
#include "sipxunit/TestUtilities.h"
#include "testlib/FileTestContext.h"
#include "utl/PluginHooks.h"
#include "os/OsConfigDb.h"

#include "net/SipMessage.h"
#include "net/SipUserAgent.h"
#include "ForwardRules.h"
#include "SipRouter.h"
#include "SubscriptionAuth.h"
              
class SubscriptionAuthTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SubscriptionAuthTest);

   CPPUNIT_TEST(nonSubscribe);
   CPPUNIT_TEST(dialogSubscribeToExternalDomain);
   CPPUNIT_TEST(dialogSubscribe_ConfigSaysDoChallenge_SingleEntry);
   CPPUNIT_TEST(dialogSubscribe_ConfigSaysDoChallenge_MultipleEntries);
   CPPUNIT_TEST(dialogSubscribe_ConfigSaysDontChallenge);
   CPPUNIT_TEST(dialogSubscribe_ExemptedTargetsSayDontChallenge_SingleEntry);
   CPPUNIT_TEST(dialogSubscribe_ExemptedTargetsSayChallenge_SingleEntry);
   CPPUNIT_TEST(dialogSubscribe_ExemptedTargetsSayDontChallenge_MultipleEntries);
   CPPUNIT_TEST(dialogSubscribe_ExemptedTargetsSayChallenge_MultipleEntries);
   CPPUNIT_TEST(dialogSubscribe_AlreadyAllowedNotChallenged);
   CPPUNIT_TEST(dialogSubscribe_AlreadyDeniedNotChallenged);
   CPPUNIT_TEST(authenticatedDialogSubscribe);
   

   CPPUNIT_TEST_SUITE_END();

public:
   static FileTestContext* TestContext;
   SubscriptionAuth* dlgevntauth;
   SipUserAgent     testUserAgent;
   SipRouter*       testSipRouter;

   void setUp()
      {
         TestContext = new FileTestContext(TEST_DATA_DIR "/example.edu",
                                           TEST_WORK_DIR "/example.edu");
         TestContext->inputFile("domain-config");
         TestContext->setSipxDir(SipXecsService::ConfigurationDirType);
      }
   
   
   SubscriptionAuthTest()
   {
      testUserAgent.setIsUserAgent(FALSE);
      testUserAgent.setUserAgentHeaderProperty("sipXecs/testproxy");

      OsConfigDb configDb;
      configDb.set("SIPX_PROXY_AUTHENTICATE_ALGORITHM", "MD5");
      configDb.set("SIPX_PROXY_HOSTPORT", "sipx.example.edu");

      testSipRouter = new SipRouter(testUserAgent, mForwardingRules, configDb);
   }

   void createAndReadySubscriptionAuthForTest( OsConfigDb& configDb )
   {
      CPPUNIT_ASSERT((dlgevntauth = dynamic_cast<SubscriptionAuth*>(getAuthPlugin("dlgEvnt"))));
      dlgevntauth->announceAssociatedSipRouter( testSipRouter );
      dlgevntauth->readConfig( configDb );
   }
   
   void destroySubscriptionAuthForTest() 
   {
      delete dlgevntauth;
   }

   void tearDown()
   {
      destroySubscriptionAuthForTest();
   }

   // Test that a non SUBSCRIBE request is not affected
   void nonSubscribe()
   {
      OsConfigDb configDb;
      configDb.set("PACKAGES_REQUIRING_AUTHENTICATION", "dialog");
      createAndReadySubscriptionAuthForTest( configDb );             
      
      UtlString identity = ""; // no authenticated identity
      Url requestUri("sip:someone@example.edu");

      const char* message =
         "INFO sip:someone@example.edu SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:someone@example.edu;tag=totag\r\n"
         "From: Caller <sip:caller@example.edu>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 5 INFO\r\n"
         "Max-Forwards: 20\r\n"
         "Event: dialog\r\n"
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
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

      // verify that the message has not been modified
      UtlString forwardedMsg;
      ssize_t length;
      testMsg.getBytes(&forwardedMsg,&length);
      ASSERT_STR_EQUAL(message, forwardedMsg.data());
   }

   // Test that a "dialog event" SUBSCRIBE request that is not addressed to our domain are
   // challenged
   void dialogSubscribeToExternalDomain()
   {
      OsConfigDb configDb;
      configDb.set("PACKAGES_REQUIRING_AUTHENTICATION", "dialog");
      createAndReadySubscriptionAuthForTest( configDb );             
      
      UtlString identity = ""; // no authenticated identity
      Url requestUri("sip:someone@somewhere");

      const char* message =
         "SUBSCRIBE sip:someone@somewhere SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:someone@somewhere\r\n"
         "From: Caller <sip:caller@example.edu>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 SUBSCRIBE\r\n"
         "Event: dialog\r\n"
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

      UtlString method("SUBSCRIBE");
      bool bSpiralingRequest = false;
      AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;

      CPPUNIT_ASSERT(AuthPlugin::DENY
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      // Make sure we set a reason why we are challenging this request
      CPPUNIT_ASSERT(0 != rejectReason.compareTo(unmodifiedRejectReason));
   }

   // Test that a SUBSCRIBE request is challenged
   // when event package is found in PACKAGES_REQUIRING_AUTHENTICATION
   void dialogSubscribe_ConfigSaysDoChallenge_SingleEntry()
   {
      OsConfigDb configDb;
      configDb.set("PACKAGES_REQUIRING_AUTHENTICATION", "lonelypackage");
      createAndReadySubscriptionAuthForTest( configDb );             
      
      UtlString identity = ""; // no authenticated identity
      Url requestUri("sip:someone@example.edu");

      const char* message1 =
         "SUBSCRIBE sip:someone@example.edu SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:someone@example.edu\r\n"
         "From: Caller <sip:caller@somewhere>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 SUBSCRIBE\r\n"
         "Event: lonelypackage\r\n"
         "Max-Forwards: 20\r\n"
         "Contact: caller@127.0.0.1\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage testMsg1(message1, strlen(message1));

      UtlSList noRemovedRoutes;
      UtlString myRouteName("myhost.example.com");
      RouteState routeState( testMsg1, noRemovedRoutes, myRouteName );

      const char unmodifiedRejectReason[] = "unmodified";
      UtlString rejectReason(unmodifiedRejectReason);

      UtlString method("SUBSCRIBE");
      bool bSpiralingRequest = false;
      AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;


      CPPUNIT_ASSERT(AuthPlugin::DENY
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg1,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      // Make sure we set a reason why we are challenging this request
      CPPUNIT_ASSERT(0 != rejectReason.compareTo(unmodifiedRejectReason));
   }
   
   // Test that a SUBSCRIBE request is challenged
   // when event package is found in PACKAGES_REQUIRING_AUTHENTICATION
   void dialogSubscribe_ConfigSaysDoChallenge_MultipleEntries()
   {
      OsConfigDb configDb;
      configDb.set("PACKAGES_REQUIRING_AUTHENTICATION", "this that theother");
      createAndReadySubscriptionAuthForTest( configDb );             
      
      UtlString identity = ""; // no authenticated identity
      Url requestUri("sip:someone@example.edu");

      const char* message1 =
         "SUBSCRIBE sip:someone@example.edu SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:someone@example.edu\r\n"
         "From: Caller <sip:caller@somewhere>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 SUBSCRIBE\r\n"
         "Event: this\r\n"
         "Max-Forwards: 20\r\n"
         "Contact: caller@127.0.0.1\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage testMsg1(message1, strlen(message1));

      UtlSList noRemovedRoutes;
      UtlString myRouteName("myhost.example.com");
      RouteState routeState( testMsg1, noRemovedRoutes, myRouteName );

      const char unmodifiedRejectReason[] = "unmodified";
      UtlString rejectReason(unmodifiedRejectReason);

      UtlString method("SUBSCRIBE");
      bool bSpiralingRequest = false;
      AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;

      CPPUNIT_ASSERT(AuthPlugin::DENY
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg1,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      // Make sure we set a reason why we are challenging this request
      CPPUNIT_ASSERT(0 != rejectReason.compareTo(unmodifiedRejectReason));

      const char* message2 =
         "SUBSCRIBE sip:someone@example.edu SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:someone@example.edu\r\n"
         "From: Caller <sip:caller@somewhere>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 SUBSCRIBE\r\n"
         "Event: that\r\n"
         "Max-Forwards: 20\r\n"
         "Contact: caller@127.0.0.1\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage testMsg2(message2, strlen(message2));
      
      CPPUNIT_ASSERT(AuthPlugin::DENY
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg2,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      // Make sure we set a reason why we are challenging this request
      CPPUNIT_ASSERT(0 != rejectReason.compareTo(unmodifiedRejectReason));

      const char* message3 =
         "SUBSCRIBE sip:someone@example.edu SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:someone@example.edu\r\n"
         "From: Caller <sip:caller@somewhere>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 SUBSCRIBE\r\n"
         "Event: theother\r\n"
         "Max-Forwards: 20\r\n"
         "Contact: caller@127.0.0.1\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage testMsg3(message3, strlen(message3));
      CPPUNIT_ASSERT(AuthPlugin::DENY
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg3,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      // Make sure we set a reason why we are challenging this request
      CPPUNIT_ASSERT(0 != rejectReason.compareTo(unmodifiedRejectReason));
   }

   // Test that a SUBSCRIBE request is unaffected
   // when event package is not found in PACKAGES_REQUIRING_AUTHENTICATION
   void dialogSubscribe_ConfigSaysDontChallenge()
   {
      OsConfigDb configDb;
      configDb.set("PACKAGES_REQUIRING_AUTHENTICATION", "bag box trailer");
      createAndReadySubscriptionAuthForTest( configDb );             
      
      UtlString identity = ""; // no authenticated identity
      Url requestUri("sip:someone@example.edu");

      const char* message =
         "SUBSCRIBE sip:someone@example.edu SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:someone@example.edu\r\n"
         "From: Caller <sip:caller@somewhere>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 SUBSCRIBE\r\n"
         "Event: dialog\r\n"
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

      UtlString method("SUBSCRIBE");
      bool bSpiralingRequest = false;
      AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;

      CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

      // verify that the message has not been modified
      UtlString forwardedMsg;
      ssize_t length;
      testMsg.getBytes(&forwardedMsg,&length);
      ASSERT_STR_EQUAL(message, forwardedMsg.data());
   }
   
   // Test that a SUBSCRIBE request normally requiring authentication 
   // is not challenged if a previous auth plug-in already allowed it
   void dialogSubscribe_AlreadyAllowedNotChallenged()
   {
      OsConfigDb configDb;
      configDb.set("PACKAGES_REQUIRING_AUTHENTICATION", "dialog");
      createAndReadySubscriptionAuthForTest( configDb );             
      
      UtlString identity = ""; // no authenticated identity
      Url requestUri("sip:someone@example.edu");

      const char* message =
         "SUBSCRIBE sip:someone@example.edu SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:someone@example.edu\r\n"
         "From: Caller <sip:caller@somewhere>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 SUBSCRIBE\r\n"
         "Event: dialog\r\n"
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

      UtlString method("SUBSCRIBE");
      bool bSpiralingRequest = false;
      AuthPlugin::AuthResult priorResult = AuthPlugin::ALLOW;

      CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

      // verify that the message has not been modified
      UtlString forwardedMsg;
      ssize_t length;
      testMsg.getBytes(&forwardedMsg,&length);
      ASSERT_STR_EQUAL(message, forwardedMsg.data());
   }
   
   // Test that a SUBSCRIBE request normally requiring authentication 
   // is not challenged if a previous auth plug-in already denied it
   void dialogSubscribe_AlreadyDeniedNotChallenged()
   {
      OsConfigDb configDb;
      configDb.set("PACKAGES_REQUIRING_AUTHENTICATION", "dialog");
      createAndReadySubscriptionAuthForTest( configDb );             
      
      UtlString identity = ""; // no authenticated identity
      Url requestUri("sip:someone@example.edu");

      const char* message =
         "SUBSCRIBE sip:someone@example.edu SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:someone@example.edu\r\n"
         "From: Caller <sip:caller@somewhere>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 SUBSCRIBE\r\n"
         "Event: dialog\r\n"
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

      UtlString method("SUBSCRIBE");
      bool bSpiralingRequest = false;
      AuthPlugin::AuthResult priorResult = AuthPlugin::DENY;

      CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

      // verify that the message has not been modified
      UtlString forwardedMsg;
      ssize_t length;
      testMsg.getBytes(&forwardedMsg,&length);
      ASSERT_STR_EQUAL(message, forwardedMsg.data());
   }
       
   // Test that an authenticated SUBSCRIBE request normally requiring authentication 
   // is unaffected
   void authenticatedDialogSubscribe()
   {
      OsConfigDb configDb;
      configDb.set("PACKAGES_REQUIRING_AUTHENTICATION", "dialog");
      createAndReadySubscriptionAuthForTest( configDb );             
      
      UtlString identity = "caller"; // authenticated identity
      Url requestUri("sip:someone@example.edu");

      const char* message =
         "SUBSCRIBE sip:someone@example.edu SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:someone@example.edu\r\n"
         "From: Caller <sip:caller@example.edu>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 SUBSCRIBE\r\n"
         "Event: dialog\r\n"
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

      UtlString method("SUBSCRIBE");
      bool bSpiralingRequest = false;
      AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;

      CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

      // verify that the message has not been modified
      UtlString forwardedMsg;
      ssize_t length;
      testMsg.getBytes(&forwardedMsg,&length);
      ASSERT_STR_EQUAL(message, forwardedMsg.data());
   }

   void dialogSubscribe_ExemptedTargetsSayDontChallenge_SingleEntry()
   {
      OsConfigDb configDb;
      configDb.set("PACKAGES_REQUIRING_AUTHENTICATION", "dialog");
      configDb.set("TARGETS_EXEMPTED_FROM_AUTHENTICATION", "^~~.o");
      createAndReadySubscriptionAuthForTest( configDb );             
      
      UtlString identity = ""; // no authenticated identity
      Url requestUri("sip:~~robert@example.edu");

      const char* message =
         "SUBSCRIBE sip:~~robert@example.edu SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:someone@example.edu\r\n"
         "From: Caller <sip:caller@example.edu>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 SUBSCRIBE\r\n"
         "Event: dialog\r\n"
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

      UtlString method("SUBSCRIBE");
      bool bSpiralingRequest = false;
      AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;

      CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

      // verify that the message has not been modified
      UtlString forwardedMsg;
      ssize_t length;
      testMsg.getBytes(&forwardedMsg,&length);
      ASSERT_STR_EQUAL(message, forwardedMsg.data());
   }

   void dialogSubscribe_ExemptedTargetsSayChallenge_SingleEntry()
   {
      OsConfigDb configDb;
      configDb.set("PACKAGES_REQUIRING_AUTHENTICATION", "dialog");
      configDb.set("TARGETS_EXEMPTED_FROM_AUTHENTICATION", "^~~.o");
      createAndReadySubscriptionAuthForTest( configDb );             
      
      UtlString identity = ""; // no authenticated identity
      Url requestUri("sip:~~rabert@example.edu");

      const char* message =
         "SUBSCRIBE sip:~~robert@example.edu SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:someone@example.edu\r\n"
         "From: Caller <sip:caller@example.edu>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 SUBSCRIBE\r\n"
         "Event: dialog\r\n"
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

      UtlString method("SUBSCRIBE");
      bool bSpiralingRequest = false;
      AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;

      CPPUNIT_ASSERT(AuthPlugin::DENY
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      // Make sure we set a reason why we are challenging this request
      CPPUNIT_ASSERT(0 != rejectReason.compareTo(unmodifiedRejectReason));
   }
   
   void dialogSubscribe_ExemptedTargetsSayDontChallenge_MultipleEntries()
   {
      OsConfigDb configDb;
      configDb.set("PACKAGES_REQUIRING_AUTHENTICATION", "dialog");
      configDb.set("TARGETS_EXEMPTED_FROM_AUTHENTICATION", "^~~.o ab..e");
      createAndReadySubscriptionAuthForTest( configDb );             
      
      UtlString identity = ""; // no authenticated identity
      Url requestUri("sip:~~robert@example.edu");

      const char* message =
         "SUBSCRIBE sip:~~robert@example.edu SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:someone@example.edu\r\n"
         "From: Caller <sip:caller@example.edu>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 SUBSCRIBE\r\n"
         "Event: dialog\r\n"
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

      UtlString method("SUBSCRIBE");
      bool bSpiralingRequest = false;
      AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;

      CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

      // verify that the message has not been modified
      UtlString forwardedMsg;
      ssize_t length;
      testMsg.getBytes(&forwardedMsg,&length);
      ASSERT_STR_EQUAL(message, forwardedMsg.data());

      Url requestUri2("sip:123abcde123@example.edu");
      CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri2,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

      // verify that the message has not been modified
      testMsg.getBytes(&forwardedMsg,&length);
      ASSERT_STR_EQUAL(message, forwardedMsg.data());
   
   }

   void dialogSubscribe_ExemptedTargetsSayChallenge_MultipleEntries()
   {
      OsConfigDb configDb;
      configDb.set("PACKAGES_REQUIRING_AUTHENTICATION", "dialog");
      configDb.set("TARGETS_EXEMPTED_FROM_AUTHENTICATION", "^~~.o ab..e");
      createAndReadySubscriptionAuthForTest( configDb );             
      
      UtlString identity = ""; // no authenticated identity
      Url requestUri("sip:~~rabert@example.edu");

      const char* message =
         "SUBSCRIBE sip:~~rabert@example.edu SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:someone@example.edu\r\n"
         "From: Caller <sip:caller@example.edu>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 SUBSCRIBE\r\n"
         "Event: dialog\r\n"
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

      UtlString method("SUBSCRIBE");
      bool bSpiralingRequest = false;
      AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;

      CPPUNIT_ASSERT(AuthPlugin::DENY
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      // Make sure we set a reason why we are challenging this request
      CPPUNIT_ASSERT(0 != rejectReason.compareTo(unmodifiedRejectReason));

      Url requestUri2("sip:123abcdf123@example.edu");
      CPPUNIT_ASSERT(AuthPlugin::DENY
                     == dlgevntauth->authorizeAndModify(identity,
                                                        requestUri2,
                                                        routeState,
                                                        method,
                                                        priorResult,
                                                        testMsg,
                                                        bSpiralingRequest,
                                                        rejectReason
                                                        ));
      // Make sure we set a reason why we are challenging this request
      CPPUNIT_ASSERT(0 != rejectReason.compareTo(unmodifiedRejectReason));
   }
  
private:
   ForwardRules  mForwardingRules;
};

FileTestContext* SubscriptionAuthTest::TestContext;

CPPUNIT_TEST_SUITE_REGISTRATION(SubscriptionAuthTest);


