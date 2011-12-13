//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////

#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/TestCase.h"
#include "sipxunit/TestUtilities.h"
#include "sipxunit/FileTestContext.h"

#include "os/OsDefs.h"
#include "os/OsLogger.h"
#include "utl/PluginHooks.h"
#include "net/SipMessage.h"
#include "net/SipUserAgent.h"

#include "ForwardRules.h"
#include <sipxproxy/SipRouter.h>
#include <sipxproxy/AuthPlugin.h>
#include "CallerAlertInfo.h"

#define EXTERNAL_TEXT "<http://external-test-text.example.edu>"
#define INTERNAL_TEXT "<http://internal-test-text.example.edu>"

class CallerAlertInfoTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(CallerAlertInfoTest);

   CPPUNIT_TEST(testConstructor); // ! MUST BE FIRST

   CPPUNIT_TEST(testInternal);
   CPPUNIT_TEST(testExternal);
   CPPUNIT_TEST(testExisting);

   CPPUNIT_TEST_SUITE_END();

public:

   static CallerAlertInfo*  pluginTestee;
   static SipUserAgent      testUserAgent;
   static SipRouter*        testSipRouter;

   void setUp()
      {
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
         CPPUNIT_ASSERT((pluginTestee = dynamic_cast<CallerAlertInfo*>(getAuthPlugin("testee"))));

         testUserAgent.setDnsSrvTimeout(1 /* seconds */);
         testUserAgent.setMaxSrvRecords(4);
         testUserAgent.setUserAgentHeaderProperty("sipXecs/sipXproxy");

         testUserAgent.setForking(FALSE);  // Disable forking

         OsConfigDb configDb;
         configDb.set("SIPX_PROXY_AUTHENTICATE_ALGORITHM", "");
         configDb.set("SIPX_PROXY_HOSTPORT", "example.edu");

         configDb.set("EXTERNAL", EXTERNAL_TEXT);
         configDb.set("EXTERNAL_ENABLED", "1");
         configDb.set("INTERNAL", INTERNAL_TEXT);
         configDb.set("INTERNAL_ENABLED", "1");
         configDb.set("ON_EXISTING", "0");

         testSipRouter = new SipRouter(testUserAgent, mForwardingRules, configDb);
         pluginTestee->announceAssociatedSipRouter( testSipRouter );
         pluginTestee->readConfig(configDb);
      }

   ///Test an internal call
   void testInternal()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:200@example.edu");

         const char* message =
            "INVITE sip:200@example.edu SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:200@example.edu\r\n"
            "From: Caller <sip:caller@example.edu>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 2 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         UtlString routeName("example.com");
         RouteState routeState( testMsg, noRemovedRoutes, routeName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);

         UtlString method("INVITE");
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         bool bSpiralingRequest = false;

         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == pluginTestee->authorizeAndModify(identity,
                                                            requestUri,
                                                            routeState,
                                                            method,
                                                            priorResult,
                                                            testMsg,
                                                            bSpiralingRequest,
                                                            rejectReason
                                                            ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         const char* alertInfoValue = testMsg.getHeaderValue(0, "Alert-Info");
         CPPUNIT_ASSERT(alertInfoValue != NULL);
         ASSERT_STR_EQUAL(INTERNAL_TEXT, alertInfoValue);
      }

   ///Test an external call
   void testExternal()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:200@example.edu");

         const char* message =
            "INVITE sip:200@example.edu SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:200@example.edu\r\n"
            "From: Caller <sip:caller@example.com>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 2 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         UtlString routeName("example.com");
         RouteState routeState( testMsg, noRemovedRoutes, routeName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);

         UtlString method("INVITE");
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         bool bSpiralingRequest = false;

         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == pluginTestee->authorizeAndModify(identity,
                                                            requestUri,
                                                            routeState,
                                                            method,
                                                            priorResult,
                                                            testMsg,
                                                            bSpiralingRequest,
                                                            rejectReason
                                                            ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         const char* alertInfoValue = testMsg.getHeaderValue(0, "Alert-Info");
         CPPUNIT_ASSERT(alertInfoValue != NULL);
         ASSERT_STR_EQUAL(EXTERNAL_TEXT, alertInfoValue);
      }

   /// Make sure existing fields aren't overwritten
   void testExisting()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:200@example.edu");

         const char* message =
            "INVITE sip:200@example.edu SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:200@example.edu\r\n"
            "From: Caller <sip:caller@example.com>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 2 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Alert-Info: <http://www.example.com/sounds/moo.wav>\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         UtlString routeName("example.com");
         RouteState routeState( testMsg, noRemovedRoutes, routeName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);

         UtlString method("INVITE");
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         bool bSpiralingRequest = false;

         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == pluginTestee->authorizeAndModify(identity,
                                                            requestUri,
                                                            routeState,
                                                            method,
                                                            priorResult,
                                                            testMsg,
                                                            bSpiralingRequest,
                                                            rejectReason
                                                            ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         const char* alertInfoValue = testMsg.getHeaderValue(0, "Alert-Info");
         CPPUNIT_ASSERT(alertInfoValue != NULL);
         ASSERT_STR_EQUAL("<http://www.example.com/sounds/moo.wav>", alertInfoValue);
      }


private:
   ForwardRules  mForwardingRules;

};

CPPUNIT_TEST_SUITE_REGISTRATION(CallerAlertInfoTest);

CallerAlertInfo*     CallerAlertInfoTest::pluginTestee;
SipUserAgent     CallerAlertInfoTest::testUserAgent(
   PORT_NONE,
   PORT_NONE,
   PORT_NONE,
   NULL, // public IP address (not used in proxy)
   NULL, // default user (not used in proxy)
   NULL, // default SIP address (not used in proxy)
   NULL, // outbound proxy
   NULL, // directory server
   NULL, // registry server
   NULL, // auth realm
   NULL, // auth DB
   NULL, // auth user IDs
   NULL, // auth passwords
   NULL, // line mgr
   SIP_DEFAULT_RTT, // first resend timeout
   FALSE, // default to proxy transaction
   SIPUA_DEFAULT_SERVER_UDP_BUFFER_SIZE, // socket layer read buffer size
   SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE // OsServerTask message queue size
                                                );
SipRouter* CallerAlertInfoTest::testSipRouter;
