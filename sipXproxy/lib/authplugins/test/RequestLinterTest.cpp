//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/TestCase.h"
#include "sipxunit/TestUtilities.h"
#include "testlib/SipDbTestContext.h"
#include "os/OsDefs.h"
#include "os/OsConfigDb.h"
#include "utl/PluginHooks.h"
#include "net/SipMessage.h"
#include "net/SipUserAgent.h"
#include "ForwardRules.h"
#include "SipRouter.h"
#include "AuthPlugin.h"
#include "RequestLinter.h"
#include "net/SipXauthIdentity.h"

class RequestLinterTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(RequestLinterTest);

   CPPUNIT_TEST(testNoPAI);
   CPPUNIT_TEST(testPAIForOtherDomain);
   CPPUNIT_TEST(testPAIForMatchingDomain_SpiralEnded);
   CPPUNIT_TEST(testPAIForMatchingDomain_StillSpiraling);
   CPPUNIT_TEST(testMultiplePAIs_NoMatch);
   CPPUNIT_TEST(testMultiplePAIs_Match);
   CPPUNIT_TEST(testMultiplePAIs_Mixed);
   CPPUNIT_TEST_SUITE_END();

private:
   ForwardRules  mForwardingRules;
   
public:

   static FileTestContext* TestContext;
   static RequestLinter* spLinter;
   static SipUserAgent   testUserAgent;
   static SipRouter*     testSipRouter;

   void setUp()
      {
         TestContext = new FileTestContext(TEST_DATA_DIR "/mydomain.com",
                                           TEST_WORK_DIR "/mydomain.com");
         TestContext->inputFile("domain-config");
         TestContext->setSipxDir(SipXecsService::ConfigurationDirType);
      }

   UtlString identity; 
   Url requestUri;
   RequestLinterTest() :
      identity(""),
      requestUri("sip:someguy@example.com")
   {
      
      testUserAgent.setIsUserAgent(FALSE);
      testUserAgent.setUserAgentHeaderProperty("sipXecs/testproxy");
      testUserAgent.setForking(FALSE);  // Disable forking
      UtlString hostAliases("mydomainalias.com mydomainotheralias.com");
      testUserAgent.setHostAliases(hostAliases);

      OsConfigDb configDb;
      configDb.set("SIPX_PROXY_AUTHENTICATE_ALGORITHM", "MD5");
      configDb.set("SIPX_PROXY_HOSTPORT", "mydomain.com");

      testSipRouter = new SipRouter(testUserAgent, mForwardingRules, configDb);
      spLinter->announceAssociatedSipRouter( testSipRouter );

   }
   
   void testNoPAI()
   {
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage testMsg(message, strlen(message));
      
      UtlSList noRemovedRoutes;
      UtlString routeName("mydomain.com");
      RouteState routeState( testMsg, noRemovedRoutes, routeName );   
      UtlString rejectReason;
      
      CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                     == spLinter->authorizeAndModify(identity,
                                                     requestUri,
                                                     routeState,
                                                     "INVITE",
                                                     AuthPlugin::CONTINUE,
                                                     testMsg,
                                                     false,
                                                     rejectReason
                                                     ));

      ssize_t size;
      UtlString modifiedMessageText;
      testMsg.getBytes( &modifiedMessageText, &size );
      ASSERT_STR_EQUAL( message, modifiedMessageText.data() );
   }
   
   void testPAIForOtherDomain()
   {
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "P-Asserted-Identity: \"Some guy\" <sip:100@notmydomain.com>\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage testMsg(message, strlen(message));
      
      UtlSList noRemovedRoutes;
      UtlString routeName("mydomain.com");
      RouteState routeState( testMsg, noRemovedRoutes, routeName );   
      UtlString rejectReason;
      
      CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                     == spLinter->authorizeAndModify(identity,
                                                     requestUri,
                                                     routeState,
                                                     "INVITE",
                                                     AuthPlugin::CONTINUE,
                                                     testMsg,
                                                     false,
                                                     rejectReason
                                                     ));

      ssize_t size;
      UtlString modifiedMessageText;
      testMsg.getBytes( &modifiedMessageText, &size );
      ASSERT_STR_EQUAL( message, modifiedMessageText.data() );      
   }
   
   void testPAIForMatchingDomain_SpiralEnded()
   {
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "P-Asserted-Identity: \"Some guy\" <sip:100@mydomain.com>\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage testMsg(message, strlen(message));
      CPPUNIT_ASSERT( testMsg.getHeaderValue( 0, SipXauthIdentity::PAssertedIdentityHeaderName ) ); 
      
      UtlSList noRemovedRoutes;
      UtlString routeName("mydomain.com");
      RouteState routeState( testMsg, noRemovedRoutes, routeName );   
      UtlString rejectReason;
      
      CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                     == spLinter->authorizeAndModify(identity,
                                                     requestUri,
                                                     routeState,
                                                     "INVITE",
                                                     AuthPlugin::CONTINUE,
                                                     testMsg,
                                                     false,
                                                     rejectReason
                                                     ));
      // verify that PAI is gone.
      CPPUNIT_ASSERT( !testMsg.getHeaderValue( 0, SipXauthIdentity::PAssertedIdentityHeaderName ) ); 
   }
   
   void testPAIForMatchingDomain_StillSpiraling()
   {
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "P-Asserted-Identity: \"Some guy\" <sip:100@mydomain.com>\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage testMsg(message, strlen(message));
      CPPUNIT_ASSERT( testMsg.getHeaderValue( 0, SipXauthIdentity::PAssertedIdentityHeaderName ) ); 
      
      UtlSList noRemovedRoutes;
      UtlString routeName("mydomain.com");
      RouteState routeState( testMsg, noRemovedRoutes, routeName );   
      UtlString rejectReason;
      
      CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                     == spLinter->authorizeAndModify(identity,
                                                     requestUri,
                                                     routeState,
                                                     "INVITE",
                                                     AuthPlugin::CONTINUE,
                                                     testMsg,
                                                     true, // request still spiraling
                                                     rejectReason
                                                     ));
      // verify that PAI is still there because request is still spiraling.
      const char* pPAI;
      CPPUNIT_ASSERT( ( pPAI = testMsg.getHeaderValue( 0, SipXauthIdentity::PAssertedIdentityHeaderName ) ) ); 
      ASSERT_STR_EQUAL( "\"Some guy\" <sip:100@mydomain.com>", pPAI );
   }
   
   void testMultiplePAIs_NoMatch()
   {
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "P-Asserted-Identity: \"foreigner1\" <sip:100@notmydomain.com>\r\n"
         "P-Asserted-Identity: \"foreigner2\" <sip:101@alsonotmydomain.com>\r\n"
         "P-Asserted-Identity: \"foreigner3\" <sip:102@reallynotmydomain.com>\r\n"
         "P-Asserted-Identity: \"foreigner4\" <sip:103@absolutelynotmydomain.com>\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage testMsg(message, strlen(message));
      
      UtlSList noRemovedRoutes;
      UtlString routeName("mydomain.com");
      RouteState routeState( testMsg, noRemovedRoutes, routeName );   
      UtlString rejectReason;
      
      CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                     == spLinter->authorizeAndModify(identity,
                                                     requestUri,
                                                     routeState,
                                                     "INVITE",
                                                     AuthPlugin::CONTINUE,
                                                     testMsg,
                                                     false,
                                                     rejectReason
                                                     ));

      ssize_t size;
      UtlString modifiedMessageText;
      testMsg.getBytes( &modifiedMessageText, &size );
      ASSERT_STR_EQUAL( message, modifiedMessageText.data() );      
   }
   
   void testMultiplePAIs_Match()
   {
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "P-Asserted-Identity: \"Some guy\" <sip:100@mydomain.com>\r\n"
         "P-Asserted-Identity: \"Some other guy\" <sip:101@mydomainalias.com>\r\n"
         "P-Asserted-Identity: \"yet another guy\" <sip:102@mydomainotheralias.com>\r\n"
         "P-Asserted-Identity: \"you again\" <sip:103@mydomainotheralias.com>\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage testMsg(message, strlen(message));
      
      UtlSList noRemovedRoutes;
      UtlString routeName("mydomain.com");
      RouteState routeState( testMsg, noRemovedRoutes, routeName );   
      UtlString rejectReason;
      
      CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                     == spLinter->authorizeAndModify(identity,
                                                     requestUri,
                                                     routeState,
                                                     "INVITE",
                                                     AuthPlugin::CONTINUE,
                                                     testMsg,
                                                     false,
                                                     rejectReason
                                                     ));

      // verify that PAIs are gone.
      CPPUNIT_ASSERT( !testMsg.getHeaderValue( 0, SipXauthIdentity::PAssertedIdentityHeaderName ) );
   }
   
   void testMultiplePAIs_Mixed()
   {
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "P-Asserted-Identity: \"Some guy\" <sip:100@mydomain.com>\r\n"
         "P-Asserted-Identity: \"foreigner1\" <sip:101@foreigdomain1.com>\r\n"
         "P-Asserted-Identity: \"Some other guy\" <sip:101@mydomainalias.com>\r\n"
         "P-Asserted-Identity: \"foreigner2\" <sip:102@foreigdomain2.com>\r\n"
         "P-Asserted-Identity: \"foreigner3\" <sip:103@foreigdomain3.com>\r\n"
         "P-Asserted-Identity: \"yet another guy\" <sip:102@mydomainotheralias.com>\r\n"
         "P-Asserted-Identity: \"foreigner4\" <sip:104@foreigdomain4.com>\r\n"
         "P-Asserted-Identity: \"you again\" <sip:103@mydomainotheralias.com>\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage testMsg(message, strlen(message));
      
      UtlSList noRemovedRoutes;
      UtlString routeName("mydomain.com");
      RouteState routeState( testMsg, noRemovedRoutes, routeName );   
      UtlString rejectReason;
      
      CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                     == spLinter->authorizeAndModify(identity,
                                                     requestUri,
                                                     routeState,
                                                     "INVITE",
                                                     AuthPlugin::CONTINUE,
                                                     testMsg,
                                                     false,
                                                     rejectReason
                                                     ));

      // verify that foreign PAIs are still there.
      const char* pPAI;
      CPPUNIT_ASSERT( ( pPAI = testMsg.getHeaderValue( 0, SipXauthIdentity::PAssertedIdentityHeaderName ) ) ); 
      ASSERT_STR_EQUAL( "\"foreigner1\" <sip:101@foreigdomain1.com>", pPAI );
      CPPUNIT_ASSERT( ( pPAI = testMsg.getHeaderValue( 1, SipXauthIdentity::PAssertedIdentityHeaderName ) ) ); 
      ASSERT_STR_EQUAL( "\"foreigner2\" <sip:102@foreigdomain2.com>", pPAI );
      CPPUNIT_ASSERT( ( pPAI = testMsg.getHeaderValue( 2, SipXauthIdentity::PAssertedIdentityHeaderName ) ) ); 
      ASSERT_STR_EQUAL( "\"foreigner3\" <sip:103@foreigdomain3.com>", pPAI );
      CPPUNIT_ASSERT( ( pPAI = testMsg.getHeaderValue( 3, SipXauthIdentity::PAssertedIdentityHeaderName ) ) ); 
      ASSERT_STR_EQUAL( "\"foreigner4\" <sip:104@foreigdomain4.com>", pPAI );
   } 
};

CPPUNIT_TEST_SUITE_REGISTRATION(RequestLinterTest);

RequestLinter* RequestLinterTest::spLinter = dynamic_cast<RequestLinter*>(getAuthPlugin("linter"));
SipUserAgent     RequestLinterTest::testUserAgent;
SipRouter*       RequestLinterTest::testSipRouter;
FileTestContext* RequestLinterTest::TestContext;
