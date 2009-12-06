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
#include "testlib/FileTestContext.h"
#include "testlib/SipDbTestContext.h"

#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "utl/PluginHooks.h"
#include "net/SipMessage.h"
#include "net/SipUserAgent.h"
#include "sipdb/CallerAliasDB.h"

#include "ForwardRules.h"
#include "SipRouter.h"
#include "AuthPlugin.h"
#include "CallerAlias.h"

class CallerAliasTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(CallerAliasTest);

   CPPUNIT_TEST(testConstructor); // ! MUST BE FIRST

   CPPUNIT_TEST(testNoAlias);
   CPPUNIT_TEST(testUserAlias);
   CPPUNIT_TEST(testDomainAlias);

   CPPUNIT_TEST_SUITE_END();

public:

   static CallerAlias*      converter;
   static SipDbTestContext* TestDbContext;
   static SipUserAgent      testUserAgent;
   static SipRouter*        testSipRouter;

   void setUp()
      {
         TestDbContext = new SipDbTestContext(TEST_DATA_DIR "/example.edu",
                                              TEST_WORK_DIR "/example.edu");

         TestDbContext->inputFile("caller-alias.xml");
         TestDbContext->inputFile("domain-config");
         TestDbContext->setSipxDir(SipXecsService::ConfigurationDirType);
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
         CPPUNIT_ASSERT((converter = dynamic_cast<CallerAlias*>(getAuthPlugin("convert"))));

         testUserAgent.setDnsSrvTimeout(1 /* seconds */);
         testUserAgent.setMaxSrvRecords(4);
         testUserAgent.setUserAgentHeaderProperty("sipXecs/sipXproxy");

         testUserAgent.setForking(FALSE);  // Disable forking

         OsConfigDb configDb;
         configDb.set("SIPX_PROXY_AUTHENTICATE_ALGORITHM", "MD5");
         configDb.set("SIPX_PROXY_HOSTPORT", "sipx.example.edu");

         testSipRouter = new SipRouter(testUserAgent, mForwardingRules, configDb);
         converter->announceAssociatedSipRouter( testSipRouter );
      }

   //
   void testNoAlias()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:911@emergency-gw");

         const char* message =
            "INVITE sip:911@emergency-gw SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:911@emergency-gw\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
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
                        == converter->authorizeAndModify(identity,
                                                         requestUri,
                                                         routeState,
                                                         method,
                                                         priorResult,
                                                         testMsg,
                                                         bSpiralingRequest,
                                                         rejectReason
                                                         ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         routeState.update(&testMsg);

         UtlString recordRoute;
         CPPUNIT_ASSERT(testMsg.getRecordRouteField(0, &recordRoute));
         ASSERT_STR_EQUAL( "<sip:example.com;lr;sipXecs-rs=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60%21eb70d993aca7e9de5c931b25892705f0>", recordRoute );

         // Only one Record-Route header.
         CPPUNIT_ASSERT(!testMsg.getRecordRouteField(1, &recordRoute));

         // now simulate a spiral with the same message
         RouteState spiraledRouteState(testMsg, noRemovedRoutes, routeName);
         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == converter->authorizeAndModify(identity,
                                                         requestUri,
                                                         spiraledRouteState,
                                                         method,
                                                         priorResult,
                                                         testMsg,
                                                         bSpiralingRequest,
                                                         rejectReason
                                                        ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         spiraledRouteState.update(&testMsg);

         CPPUNIT_ASSERT(testMsg.getRecordRouteField(0, &recordRoute));
         ASSERT_STR_EQUAL( "<sip:example.com;lr;sipXecs-rs=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60%21eb70d993aca7e9de5c931b25892705f0>", recordRoute );

         // Only one Record-Route header.
         CPPUNIT_ASSERT(!testMsg.getRecordRouteField(1, &recordRoute));
      }

   void testUserAlias()
      {
         /*
          * Modify the initial INVITE
          */
         UtlString identity; // no authenticated identity
         Url requestUri("sip:target@example.org");

         const char* message =
            "INVITE sip:target@example.org SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:target@example.org\r\n"
            "From: sip:301@example.edu;tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: 301@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "testUserAlias: testMsg\n%s",
                       message
                       );

         UtlSList noRemovedRoutes;
         UtlString routeName("sipx.example.edu");
         RouteState routeState( testMsg, noRemovedRoutes, routeName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);

         UtlString method("INVITE");
         bool bSpiralingRequest = false;
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;

         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == converter->authorizeAndModify(identity,
                                                         requestUri,
                                                         routeState,
                                                         method,
                                                         priorResult,
                                                         testMsg,
                                                         bSpiralingRequest,
                                                         rejectReason
                                                         ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         UtlString modifiedFrom;
         testMsg.getFromField(&modifiedFrom);
         ASSERT_STR_EQUAL("\"John Doe\"<sip:john.doe@example.edu>;tag=30543f3483e1cb11ecb40866edd3295b", modifiedFrom);

         UtlString unmodifiedTo;
         testMsg.getToField(&unmodifiedTo);
         ASSERT_STR_EQUAL("sip:target@example.org", unmodifiedTo);

         routeState.update(&testMsg);

         UtlString recordRoute;
         CPPUNIT_ASSERT(testMsg.getRecordRouteField(0, &recordRoute));
         ASSERT_STR_EQUAL( "<sip:sipx.example.edu;lr;sipXecs-rs=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60.convert%2Aa%7EIkpvaG4gRG9lIjxzaXA6am9obi5kb2VAZXhhbXBsZS5lZHU%27O3RhZz0%60.convert%2Aao%7ENDE%60.convert%2Ac%7Ec2lwOjMwMUBleGFtcGxlLmVkdTt0YWc9.convert%2Aco%7EMjQ%60%21af22edde80ff606ad56c3cdcee2718cf>", recordRoute );

         /*
          * Now see if the ACK is modified when it comes through
          * (it is just a caller->callee in-dialog request for this plugin)
          */
         const char* ackMessage =
            "INVITE sip:target@example.org SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:target@example.org;tag=4711018y5y1-1-1\r\n"
            "From: sip:301@example.edu;tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: 301@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage ackMsg(ackMessage, strlen(ackMessage));

         UtlSList removedRoutes;
         removedRoutes.insert(&recordRoute);

         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "testUserAlias: ackMsg\n%s",
                       ackMessage
                       );

         RouteState ackRouteState( ackMsg, removedRoutes, routeName );

         method = "ACK";

         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == converter->authorizeAndModify(identity,
                                                         requestUri,
                                                         ackRouteState,
                                                         method,
                                                         priorResult,
                                                         ackMsg,
                                                         bSpiralingRequest,
                                                         rejectReason
                                                         ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         ackMsg.getFromField(&modifiedFrom);
         ASSERT_STR_EQUAL("\"John Doe\"<sip:john.doe@example.edu>;tag=30543f3483e1cb11ecb40866edd3295b", modifiedFrom);

         ackMsg.getToField(&unmodifiedTo);
         ASSERT_STR_EQUAL("sip:target@example.org;tag=4711018y5y1-1-1", unmodifiedTo);

         /*
          * See if an in-dialog request to caller is modified
          */
         const char* reverseMessage =
            "INFO 301@127.0.0.1 SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: \"John Doe\"<sip:john.doe@example.edu>;tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "From: sip:target@example.org;tag=4711018y5y1-1-1\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 5 INFO\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: target@127.0.0.2\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage reverseMsg(reverseMessage, strlen(reverseMessage));

         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "testUserAlias: reverseMsg\n%s",
                       reverseMessage
                       );

         UtlString infoMethod("INFO");

         RouteState reverseRouteState( reverseMsg, removedRoutes, routeName );

         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == converter->authorizeAndModify(identity,
                                                         requestUri,
                                                         reverseRouteState,
                                                         infoMethod,
                                                         priorResult,
                                                         reverseMsg,
                                                         bSpiralingRequest,
                                                         rejectReason
                                                         ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         UtlString unmodifiedFrom;
         reverseMsg.getFromField(&unmodifiedFrom);
         ASSERT_STR_EQUAL("sip:target@example.org;tag=4711018y5y1-1-1", unmodifiedFrom);

         UtlString modifiedTo;
         reverseMsg.getToField(&modifiedTo);
         ASSERT_STR_EQUAL("sip:301@example.edu;tag=30543f3483e1cb11ecb40866edd3295b", modifiedTo);

      }


   void testDomainAlias()
      {
         /*
          * Modify the initial INVITE
          */
         UtlString identity; // no authenticated identity
         Url requestUri("sip:target@pstn-gateway.example.edu");

         const char* message =
            "INVITE sip:target@pstn-gateway.example.edu SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:target@example.edu\r\n"
            "From: sip:505@example.edu;tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: 505@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "testDomainAlias: testMsg\n%s",
                       message
                       );

         UtlSList noRemovedRoutes;
         UtlString routeName("sipx.example.edu");
         RouteState routeState( testMsg, noRemovedRoutes, routeName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);

         UtlString method("INVITE");
         bool bSpiralingRequest = false;
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;

         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == converter->authorizeAndModify(identity,
                                                         requestUri,
                                                         routeState,
                                                         method,
                                                         priorResult,
                                                         testMsg,
                                                         bSpiralingRequest,
                                                         rejectReason
                                                         ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         UtlString modifiedFrom;
         testMsg.getFromField(&modifiedFrom);
         ASSERT_STR_EQUAL("\"Example University\"<sip:9785551200@example.edu>;tag=30543f3483e1cb11ecb40866edd3295b", modifiedFrom);

         UtlString unmodifiedTo;
         testMsg.getToField(&unmodifiedTo);
         ASSERT_STR_EQUAL("sip:target@example.edu", unmodifiedTo);

         routeState.update(&testMsg);

         UtlString recordRoute;
         CPPUNIT_ASSERT(testMsg.getRecordRouteField(0, &recordRoute));
         ASSERT_STR_EQUAL( "<sip:sipx.example.edu;lr;sipXecs-rs=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60.convert%2Aa%7EIkV4YW1wbGUgVW5pdmVyc2l0eSI8c2lwOjk3ODU1NTEyMDBAZXhhbXBsZS5lZHU%27O3RhZz0%60.convert%2Aao%7ENTM%60.convert%2Ac%7Ec2lwOjUwNUBleGFtcGxlLmVkdTt0YWc9.convert%2Aco%7EMjQ%60%212374a8f6e8cad3d19d2e7a597f7fbc48>", recordRoute );

         /*
          * Now see if the ACK is modified when it comes through
          * (it is just a caller->callee in-dialog request for this plugin)
          */
         const char* ackMessage =
            "ACK sip:target@example.org SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:target@example.org;tag=4711018y5y1-1-1\r\n"
            "From: sip:505@example.edu;tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: 301@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage ackMsg(ackMessage, strlen(ackMessage));

         UtlSList removedRoutes;
         removedRoutes.insert(&recordRoute);

         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "testDomainAlias: ackMsg\n%s",
                       ackMessage
                       );

         RouteState ackRouteState( ackMsg, removedRoutes, routeName );

         method = "ACK";

         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == converter->authorizeAndModify(identity,
                                                         requestUri,
                                                         ackRouteState,
                                                         method,
                                                         priorResult,
                                                         ackMsg,
                                                         bSpiralingRequest,
                                                         rejectReason
                                                         ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         ackMsg.getFromField(&modifiedFrom);
         ASSERT_STR_EQUAL("\"Example University\"<sip:9785551200@example.edu>;tag=30543f3483e1cb11ecb40866edd3295b", modifiedFrom);

         ackMsg.getToField(&unmodifiedTo);
         ASSERT_STR_EQUAL("sip:target@example.org;tag=4711018y5y1-1-1", unmodifiedTo);

         /*
          * See if an in-dialog request to caller is modified
          */
         const char* reverseMessage =
            "INFO 301@127.0.0.1 SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:505@example.edu;tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "From: sip:target@example.org;tag=4711018y5y1-1-1\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 5 INFO\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: target@127.0.0.2\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage reverseMsg(reverseMessage, strlen(reverseMessage));

         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "testDomainAlias: reverseMsg\n%s",
                       reverseMessage
                       );

         RouteState reverseRouteState( reverseMsg, removedRoutes, routeName );

         method = "INFO";

         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == converter->authorizeAndModify(identity,
                                                         requestUri,
                                                         reverseRouteState,
                                                         method,
                                                         priorResult,
                                                         reverseMsg,
                                                         bSpiralingRequest,
                                                         rejectReason
                                                         ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         UtlString unmodifiedFrom;
         reverseMsg.getFromField(&unmodifiedFrom);
         ASSERT_STR_EQUAL("sip:target@example.org;tag=4711018y5y1-1-1", unmodifiedFrom);

         UtlString modifiedTo;
         reverseMsg.getToField(&modifiedTo);
         ASSERT_STR_EQUAL("sip:505@example.edu;tag=30543f3483e1cb11ecb40866edd3295b", modifiedTo);
      }

private:
   ForwardRules  mForwardingRules;

};

CPPUNIT_TEST_SUITE_REGISTRATION(CallerAliasTest);

CallerAlias*     CallerAliasTest::converter;
SipUserAgent     CallerAliasTest::testUserAgent(
   PORT_NONE,
   PORT_NONE,
   PORT_NONE,
   NULL, // public IP address (not used in proxy)
   NULL, // default user (not used in proxy)
   NULL, // default SIP address (not used in proxy)
   NULL, // outbound proxy
   NULL, // directory server
   NULL, // registry server
   NULL, // auth scheme
   NULL, //auth realm
   NULL, // auth DB
   NULL, // auth user IDs
   NULL, // auth passwords
   NULL, // line mgr
   SIP_DEFAULT_RTT, // first resend timeout
   FALSE, // default to proxy transaction
   SIPUA_DEFAULT_SERVER_UDP_BUFFER_SIZE, // socket layer read buffer size
   SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE // OsServerTask message queue size
                                                );
SipRouter* CallerAliasTest::testSipRouter;
SipDbTestContext* CallerAliasTest::TestDbContext;
