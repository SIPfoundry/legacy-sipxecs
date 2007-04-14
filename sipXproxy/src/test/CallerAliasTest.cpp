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
#include "testlib/SipDbTestContext.h"

#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "utl/PluginHooks.h"
#include "net/SipMessage.h"
#include "net/SipUserAgent.h"
#include "sipdb/CallerAliasDB.h"

#include "sipauthproxy/SipAaa.h"
#include "sipauthproxy/AuthPlugin.h"
#include "sipauthproxy/CallerAlias.h"

class CallerAliasTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(CallerAliasTest);

   CPPUNIT_TEST(testConstructor); // ! MUST BE FIRST

   CPPUNIT_TEST(testNoAlias);
   CPPUNIT_TEST(testUserAlias);
   CPPUNIT_TEST(testDomainAlias);

   CPPUNIT_TEST_SUITE_END();

public:

   static CallerAlias*     converter;
   static SipDbTestContext TestDbContext;
   static SipUserAgent     testUserAgent;
   static SipAaa*          testSipAaa;
   
   void setUp()
      {
         TestDbContext.inputFile("caller-alias.xml");

         RouteState::setSecret("fixed"); // force invariant signatures
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

         testUserAgent.setIsUserAgent(FALSE);

         testUserAgent.setDnsSrvTimeout(1 /* seconds */);
         testUserAgent.setMaxSrvRecords(4);
         testUserAgent.setUserAgentHeaderProperty("sipX/authproxy");

         testUserAgent.setForking(FALSE);  // Disable forking

         UtlString hostAliases("sipx.example.edu sipx.example.edu:5080");
         
         testUserAgent.setHostAliases(hostAliases);

         OsConfigDb configDb;
         configDb.set("SIP_AUTHPROXY_AUTHENTICATE_ALGORITHM", "MD5");
         configDb.set("SIP_AUTHPROXY_DOMAIN_NAME", "example.edu");
         configDb.set("SIP_AUTHPROXY_AUTHENTICATE_REALM", "example.edu");
         configDb.set("SIP_AUTHPROXY_ROUTE_NAME", "sipx.example.edu");

         testSipAaa = new SipAaa(testUserAgent, configDb);
      }

   // 
   void testNoAlias()
      {
         OsConfigDb configDb;
         
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
         RouteState routeState( testMsg, noRemovedRoutes );
 
         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == converter->authorizeAndModify(testSipAaa,
                                                         identity,
                                                         requestUri,
                                                         routeState,
                                                         testMsg,
                                                         rejectReason
                                                         ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         UtlString routeName("example.com");
         routeState.update(&testMsg, routeName);

         UtlString recordRoute;
         CPPUNIT_ASSERT(testMsg.getRecordRouteField(0, &recordRoute));
         ASSERT_STR_EQUAL( "<sip:example.com;lr;sipX-route=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60%21cd701fdee3c04a4e1bb9567cf6ef1d06>", recordRoute );

         // Only one Record-Route header.
         CPPUNIT_ASSERT(!testMsg.getRecordRouteField(1, &recordRoute));

         // now simulate a spiral with the same message
         RouteState spiraledRouteState(testMsg, noRemovedRoutes);
         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == converter->authorizeAndModify(testSipAaa,
                                                         identity,
                                                        requestUri,
                                                        spiraledRouteState,
                                                        testMsg,
                                                        rejectReason
                                                        ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         spiraledRouteState.update(&testMsg, routeName);

         CPPUNIT_ASSERT(testMsg.getRecordRouteField(0, &recordRoute));
         ASSERT_STR_EQUAL( "<sip:example.com;lr;sipX-route=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60%21cd701fdee3c04a4e1bb9567cf6ef1d06>", recordRoute );

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
         RouteState routeState( testMsg, noRemovedRoutes );
 
         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == converter->authorizeAndModify(testSipAaa,
                                                         identity,
                                                         requestUri,
                                                         routeState,
                                                         testMsg,
                                                         rejectReason
                                                         ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         UtlString modifiedFrom;
         testMsg.getFromField(&modifiedFrom);
         ASSERT_STR_EQUAL("\"John Doe\"<sip:john.doe@example.edu>;tag=30543f3483e1cb11ecb40866edd3295b", modifiedFrom);
         
         UtlString unmodifiedTo;
         testMsg.getToField(&unmodifiedTo);
         ASSERT_STR_EQUAL("sip:target@example.org", unmodifiedTo);
         
         UtlString routeName("sipx.example.edu");
         routeState.update(&testMsg, routeName);

         UtlString recordRoute;
         CPPUNIT_ASSERT(testMsg.getRecordRouteField(0, &recordRoute));
         ASSERT_STR_EQUAL( "<sip:sipx.example.edu;lr;sipX-route=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60.convert%2Aalias%7EIkpvaG4gRG9lIjxzaXA6am9obi5kb2VAZXhhbXBsZS5lZHU%27O3RhZz0zMDU0M2YzNDgzZTFjYjExZWNiNDA4NjZlZGQzMjk1Yg%60%60.convert%2Acaller%7Ec2lwOjMwMUBleGFtcGxlLmVkdTt0YWc9MzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60%21241ddd64d5ed940efc621a09b7e81bc3>", recordRoute );

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

         RouteState ackRouteState( ackMsg, removedRoutes );

         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == converter->authorizeAndModify(testSipAaa,
                                                         identity,
                                                         requestUri,
                                                         ackRouteState,
                                                         ackMsg,
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

         RouteState reverseRouteState( reverseMsg, removedRoutes );

         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == converter->authorizeAndModify(testSipAaa,
                                                         identity,
                                                         requestUri,
                                                         reverseRouteState,
                                                         reverseMsg,
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
         RouteState routeState( testMsg, noRemovedRoutes );
 
         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == converter->authorizeAndModify(testSipAaa,
                                                         identity,
                                                         requestUri,
                                                         routeState,
                                                         testMsg,
                                                         rejectReason
                                                         ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         UtlString modifiedFrom;
         testMsg.getFromField(&modifiedFrom);
         ASSERT_STR_EQUAL("\"Example University\"<sip:9785551200@example.edu>;tag=30543f3483e1cb11ecb40866edd3295b", modifiedFrom);
         
         UtlString unmodifiedTo;
         testMsg.getToField(&unmodifiedTo);
         ASSERT_STR_EQUAL("sip:target@example.edu", unmodifiedTo);
         
         UtlString routeName("sipx.example.edu");
         routeState.update(&testMsg, routeName);

         UtlString recordRoute;
         CPPUNIT_ASSERT(testMsg.getRecordRouteField(0, &recordRoute));
         ASSERT_STR_EQUAL( "<sip:sipx.example.edu;lr;sipX-route=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60.convert%2Aalias%7EIkV4YW1wbGUgVW5pdmVyc2l0eSI8c2lwOjk3ODU1NTEyMDBAZXhhbXBsZS5lZHU%27O3RhZz0zMDU0M2YzNDgzZTFjYjExZWNiNDA4NjZlZGQzMjk1Yg%60%60.convert%2Acaller%7Ec2lwOjUwNUBleGFtcGxlLmVkdTt0YWc9MzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60%2130f04e18ae80f33f6da51d4d3ce607a2>", recordRoute );

         /*
          * Now see if the ACK is modified when it comes through
          * (it is just a caller->callee in-dialog request for this plugin)
          */
         const char* ackMessage =
            "INVITE sip:target@example.org SIP/2.0\r\n"
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

         RouteState ackRouteState( ackMsg, removedRoutes );

         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == converter->authorizeAndModify(testSipAaa,
                                                         identity,
                                                         requestUri,
                                                         ackRouteState,
                                                         ackMsg,
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

         RouteState reverseRouteState( reverseMsg, removedRoutes );

         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == converter->authorizeAndModify(testSipAaa,
                                                         identity,
                                                         requestUri,
                                                         reverseRouteState,
                                                         reverseMsg,
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

};

CPPUNIT_TEST_SUITE_REGISTRATION(CallerAliasTest);

CallerAlias*     CallerAliasTest::converter;
SipDbTestContext CallerAliasTest::TestDbContext(TEST_DATA_DIR, TEST_DIR "/calleralias_context");
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
   NULL, // nat ping URL
   0, // nat ping frequency
   "PING", // nat ping method
   NULL, // line mgr
   SIP_DEFAULT_RTT, // first resend timeout
   TRUE, // default to UA transaction
   SIPUA_DEFAULT_SERVER_UDP_BUFFER_SIZE, // socket layer read buffer size
   SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE // OsServerTask message queue size
                                                );
SipAaa* CallerAliasTest::testSipAaa;
