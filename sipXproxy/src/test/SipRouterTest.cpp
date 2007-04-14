// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include "sipforkingproxy/ForwardRules.h"
#include "sipforkingproxy/SipRouter.h"

/**
 * Unit test for SipRouter::proxyMessage
 *
 *  These tests use only loose routes because the adjustment of a strict-routed message into
 *  a loose-routed message is tested in sipXtackLib/src/test/net/SipProxyMessageTest
 *  Similarly, the detailed testing of matching for forwarding rules is done in the
 *  ForwardRulesTest in this directory.
 */
class SipRouterTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipRouterTest);

   CPPUNIT_TEST(testGenericProxy);
   CPPUNIT_TEST(testNoMaxForwards);
   CPPUNIT_TEST(testRecordRouteProxy);
   CPPUNIT_TEST(testRecordRouteOrder);
   CPPUNIT_TEST(testAuthProxy);
   CPPUNIT_TEST(testAuthProxyWithRoute);
   CPPUNIT_TEST(testAuthProxyWithRecordRoute);
   CPPUNIT_TEST(testAliasRoute);
   CPPUNIT_TEST(testAliasRouteWithAuth);
   CPPUNIT_TEST(testAliasRouteWithRecord);
   CPPUNIT_TEST(testNoAliasRouted);
   CPPUNIT_TEST(testNoAliasRoutedWithAuth);
   CPPUNIT_TEST(testNoAliasRoutedWithAuthRecorded);

   CPPUNIT_TEST_SUITE_END();

private:

   SipUserAgent* mUserAgent;
   ForwardRules  mForwardingRules;

   static const char* VoiceMail;
   static const char* MediaServer;
   static const char* LocalHost;
   
public:

   void setUp()
      {
         // Construct a SipUserAgent to provide the isMyHostAlias recognizer

         mUserAgent = new SipUserAgent(SIP_PORT, // udp port
                                       SIP_PORT, // tcp port
                                       -1,       // tls port
                                       "127.0.0.2" // public address
                                       );

         UtlString internalDomainAlias("example.com:5060");
         mUserAgent->setHostAliases(internalDomainAlias);

         UtlString internalHostAlias("internal.example.com:5060");
         mUserAgent->setHostAliases(internalHostAlias);

         UtlString externalAlias("external.example.net:5060");
         mUserAgent->setHostAliases(externalAlias);

         UtlString rulesFile(TEST_DATA_DIR "rulesdata/routing.xml");
         mForwardingRules.loadMappings( rulesFile, MediaServer, VoiceMail, LocalHost );
      }

   void tearDown()
      {
         delete mUserAgent;
      }
   
   void testGenericProxy()
      {
         SipRouter sipRouter(*mUserAgent,
                             mForwardingRules,
                             false, // no auth proxy
                             NULL,
                             false  // no record route
                             );
         
         const char* message =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));
         
         CPPUNIT_ASSERT(sipRouter.proxyMessage(testMsg));

         UtlString proxiedMsg;
         int msgLen;
         testMsg.getBytes(&proxiedMsg, &msgLen);

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@somewhere.com", requestUri.data());
         
         UtlString noRoute;
         CPPUNIT_ASSERT( !testMsg.getRouteUri(0, &noRoute) );
         
         UtlString recordRoute;
         CPPUNIT_ASSERT( ! testMsg.getRecordRouteUri(0, &recordRoute) );
         
         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }

   void testNoMaxForwards()
      {
         SipRouter sipRouter(*mUserAgent,
                             mForwardingRules,
                             false, // no auth proxy
                             NULL,
                             false  // no record route
                             );
         
         const char* message =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         
         CPPUNIT_ASSERT(sipRouter.proxyMessage(testMsg));

         UtlString proxiedMsg;
         int msgLen;
         testMsg.getBytes(&proxiedMsg, &msgLen);

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@somewhere.com", requestUri.data());
         
         UtlString noRoute;
         CPPUNIT_ASSERT( !testMsg.getRouteUri(0, &noRoute) );
         
         UtlString recordRoute;
         CPPUNIT_ASSERT( ! testMsg.getRecordRouteUri(0, &recordRoute) );
         
         int maxForwards;
         CPPUNIT_ASSERT(   testMsg.getMaxForwards(maxForwards)
                        && maxForwards == SIP_DEFAULT_MAX_FORWARDS);
      }   

   
   void testRecordRouteProxy()
      {
         SipRouter sipRouter(*mUserAgent,
                             mForwardingRules,
                             false, // no auth proxy
                             NULL,
                             true  // add record route
                             );
         
         const char* message =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         
         CPPUNIT_ASSERT(sipRouter.proxyMessage(testMsg));

         UtlString proxiedMsg;
         int msgLen;
         testMsg.getBytes(&proxiedMsg, &msgLen);

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@somewhere.com", requestUri.data());
         
         UtlString topRoute;
         CPPUNIT_ASSERT( ! testMsg.getRouteUri(0, &topRoute) );
         
         UtlString recordRoute;
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri(0, &recordRoute) );
         ASSERT_STR_EQUAL("<sip:127.0.0.2;lr>", recordRoute.data());
         
         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }   

   void testRecordRouteOrder()
      {
         SipRouter sipRouter(*mUserAgent,
                             mForwardingRules,
                             false, // no auth proxy
                             NULL,
                             true  // add record route
                             );
         
         const char* message =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Record-Route: <sip:first.example.net;lr>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         
         CPPUNIT_ASSERT(sipRouter.proxyMessage(testMsg));

         UtlString proxiedMsg;
         int msgLen;
         testMsg.getBytes(&proxiedMsg, &msgLen);

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@somewhere.com", requestUri.data());
         
         UtlString topRoute;
         CPPUNIT_ASSERT( ! testMsg.getRouteUri(0, &topRoute) );
         
         // Record-Route: <sip:127.0.0.2;lr>, <sip:first.example.net;lr>

         UtlString myRecordRoute;
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri(0, &myRecordRoute) );
         ASSERT_STR_EQUAL("<sip:127.0.0.2;lr>", myRecordRoute.data());
         
         UtlString existingRecordRoute;
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri(1, &existingRecordRoute) );
         ASSERT_STR_EQUAL("<sip:first.example.net;lr>", existingRecordRoute.data());
         
         UtlString nomoreRecordRoute;
         CPPUNIT_ASSERT( ! testMsg.getRecordRouteUri(2, &nomoreRecordRoute) );
        
         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }   

   void testAuthProxy()
      {
         SipRouter sipRouter(*mUserAgent,
                             mForwardingRules,
                             true, // use auth proxy
                             "authproxy.example.com",
                             false  // no record route
                             );
         
         const char* message =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         
         CPPUNIT_ASSERT(sipRouter.proxyMessage(testMsg));

         UtlString proxiedMsg;
         int msgLen;
         testMsg.getBytes(&proxiedMsg, &msgLen);

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@somewhere.com", requestUri.data());
         
         UtlString topRoute;
         CPPUNIT_ASSERT( testMsg.getRouteUri(0, &topRoute) );
         ASSERT_STR_EQUAL("<sip:authproxy.example.com;lr>", topRoute.data());
         
         UtlString noRoute;
         CPPUNIT_ASSERT( !testMsg.getRouteUri(1, &noRoute) );
         
         UtlString recordRoute;
         CPPUNIT_ASSERT( ! testMsg.getRecordRouteUri(0, &recordRoute) );
         
         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }

   void testAuthProxyWithRoute()
      {
         SipRouter sipRouter(*mUserAgent,
                             mForwardingRules,
                             true, // use auth proxy
                             "authproxy.example.com",
                             false  // no record route
                             );
         
         const char* message =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Route: <sip:proxy.somewhere.com;lr>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         
         CPPUNIT_ASSERT(sipRouter.proxyMessage(testMsg));

         UtlString proxiedMsg;
         int msgLen;
         testMsg.getBytes(&proxiedMsg, &msgLen);

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@somewhere.com", requestUri.data());
         
         UtlString topRoute;
         CPPUNIT_ASSERT( testMsg.getRouteUri(0, &topRoute) );
         ASSERT_STR_EQUAL("<sip:authproxy.example.com;lr>", topRoute.data());
         
         UtlString nextRoute;
         CPPUNIT_ASSERT( testMsg.getRouteUri(1, &nextRoute) );
         ASSERT_STR_EQUAL("<sip:proxy.somewhere.com;lr>", nextRoute.data());
         
         UtlString noRoute;
         CPPUNIT_ASSERT( !testMsg.getRouteUri(2, &noRoute) );
         
         UtlString recordRoute;
         CPPUNIT_ASSERT( ! testMsg.getRecordRouteUri(0, &recordRoute) );
         
         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }


   void testAuthProxyWithRecordRoute()
      {
         SipRouter sipRouter(*mUserAgent,
                             mForwardingRules,
                             true, // use auth proxy
                             "authproxy.example.com",
                             true  // add record route
                             );
         
         const char* message =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         
         CPPUNIT_ASSERT(sipRouter.proxyMessage(testMsg));

         UtlString proxiedMsg;
         int msgLen;
         testMsg.getBytes(&proxiedMsg, &msgLen);

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@somewhere.com", requestUri.data());
         
         UtlString topRoute;
         CPPUNIT_ASSERT( testMsg.getRouteUri(0, &topRoute) );
         ASSERT_STR_EQUAL("<sip:authproxy.example.com;lr>", topRoute.data());
         
         UtlString noRoute;
         CPPUNIT_ASSERT( !testMsg.getRouteUri(1, &noRoute) );
         
         UtlString recordRoute;
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri(0, &recordRoute) );
         ASSERT_STR_EQUAL("<sip:127.0.0.2;lr>", recordRoute.data());
         
         UtlString noRecordRoute;
         CPPUNIT_ASSERT( !testMsg.getRouteUri(1, &noRecordRoute) );
         
         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }

   void testAliasRoute()
      {
         SipRouter sipRouter(*mUserAgent,
                             mForwardingRules,
                             false, // no auth proxy
                             NULL,
                             false  // no record route
                             );
         
         const char* message =
            "INVITE sip:user@external.example.net SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@external.example.net\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         
         CPPUNIT_ASSERT(sipRouter.proxyMessage(testMsg));

         // UtlString proxiedMsg;
         // int msgLen;
         // testMsg.getBytes(&proxiedMsg, &msgLen);
         // printf("In:\n%s\nOut:\n%s\n", message, proxiedMsg.data());

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@external.example.net", requestUri.data());

         UtlString topRoute;
         CPPUNIT_ASSERT( testMsg.getRouteUri(0, &topRoute) );
         ASSERT_STR_EQUAL("<sip:registrar.example.com;lr>", topRoute.data());
         
         UtlString noRoute;
         CPPUNIT_ASSERT( !testMsg.getRouteUri(1, &noRoute) );
         
         UtlString recordRoute;
         CPPUNIT_ASSERT( ! testMsg.getRecordRouteUri(0, &recordRoute) );
         
         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }

   void testAliasRouteWithAuth()
      {
         SipRouter sipRouter(*mUserAgent,
                             mForwardingRules,
                             true, // use auth proxy
                             "authproxy.example.com",
                             false  // no record route
                             );
         
         const char* message =
            "INVITE sip:user@external.example.net SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@external.example.net\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         
         CPPUNIT_ASSERT(sipRouter.proxyMessage(testMsg));

         // UtlString proxiedMsg;
         // int msgLen;
         // testMsg.getBytes(&proxiedMsg, &msgLen);
         // printf("In:\n%s\nOut:\n%s\n", message, proxiedMsg.data());

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@external.example.net", requestUri.data());

         // auth proxy is not added because this is mapped

         UtlString topRoute;
         CPPUNIT_ASSERT( testMsg.getRouteUri(0, &topRoute) );
         ASSERT_STR_EQUAL("<sip:registrar.example.com;lr>", topRoute.data());
         
         UtlString noRoute;
         CPPUNIT_ASSERT( !testMsg.getRouteUri(1, &noRoute) );
         
         UtlString recordRoute;
         CPPUNIT_ASSERT( ! testMsg.getRecordRouteUri(0, &recordRoute) );
         
         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }

   void testAliasRouteWithRecord()
      {
         SipRouter sipRouter(*mUserAgent,
                             mForwardingRules,
                             true, // use auth proxy
                             "authproxy.example.com",
                             true  // add record route
                             );
         
         const char* message =
            "INVITE sip:user@external.example.net SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@external.example.net\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         
         CPPUNIT_ASSERT(sipRouter.proxyMessage(testMsg));

         // UtlString proxiedMsg;
         // int msgLen;
         // testMsg.getBytes(&proxiedMsg, &msgLen);
         // printf("In:\n%s\nOut:\n%s\n", message, proxiedMsg.data());

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@external.example.net", requestUri.data());

         // auth proxy is not added because this is mapped

         UtlString topRoute;
         CPPUNIT_ASSERT( testMsg.getRouteUri(0, &topRoute) );
         ASSERT_STR_EQUAL("<sip:registrar.example.com;lr>", topRoute.data());
         
         UtlString noRoute;
         CPPUNIT_ASSERT( !testMsg.getRouteUri(1, &noRoute) );
         
         UtlString recordRoute;
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri(0, &recordRoute) );
         ASSERT_STR_EQUAL("<sip:127.0.0.2;lr>", recordRoute.data());
         
         UtlString noRecordRoute;
         CPPUNIT_ASSERT( ! testMsg.getRecordRouteUri(1, &recordRoute) );
         
         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }

   void testNoAliasRouted()
      {
         SipRouter sipRouter(*mUserAgent,
                             mForwardingRules,
                             false, // no auth proxy
                             NULL,
                             false  // no record route
                             );
         
         const char* message =
            "INVITE sip:user@external.example.net SIP/2.0\r\n"
            "Route: <sip:somewhere.net;lr>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@external.example.net\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         
         CPPUNIT_ASSERT(sipRouter.proxyMessage(testMsg));

         // UtlString proxiedMsg;
         // int msgLen;
         // testMsg.getBytes(&proxiedMsg, &msgLen);
         // printf("In:\n%s\nOut:\n%s\n", message, proxiedMsg.data());

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@external.example.net", requestUri.data());

         UtlString topRoute;
         CPPUNIT_ASSERT( testMsg.getRouteUri(0, &topRoute) );
         ASSERT_STR_EQUAL("<sip:somewhere.net;lr>", topRoute.data());
         
         UtlString noRoute;
         CPPUNIT_ASSERT( !testMsg.getRouteUri(1, &noRoute) );
         
         UtlString recordRoute;
         CPPUNIT_ASSERT( ! testMsg.getRecordRouteUri(0, &recordRoute) );
         
         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }

   void testNoAliasRoutedWithAuth()
      {
         SipRouter sipRouter(*mUserAgent,
                             mForwardingRules,
                             true, // use auth proxy
                             "authproxy.example.com",
                             false  // no record route
                             );
         
         const char* message =
            "INVITE sip:user@external.example.net SIP/2.0\r\n"
            "Route: <sip:somewhere.net;lr>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@external.example.net\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         
         CPPUNIT_ASSERT(sipRouter.proxyMessage(testMsg));

         // UtlString proxiedMsg;
         // int msgLen;
         // testMsg.getBytes(&proxiedMsg, &msgLen);
         // printf("In:\n%s\nOut:\n%s\n", message, proxiedMsg.data());

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@external.example.net", requestUri.data());

         UtlString topRoute;
         CPPUNIT_ASSERT( testMsg.getRouteUri(0, &topRoute) );
         ASSERT_STR_EQUAL("<sip:authproxy.example.com;lr>", topRoute.data());
         
         UtlString nextRoute;
         CPPUNIT_ASSERT( testMsg.getRouteUri(1, &nextRoute) );
         ASSERT_STR_EQUAL("<sip:somewhere.net;lr>", nextRoute.data());
         
         UtlString noRoute;
         CPPUNIT_ASSERT( !testMsg.getRouteUri(2, &noRoute) );
         
         UtlString recordRoute;
         CPPUNIT_ASSERT( ! testMsg.getRecordRouteUri(0, &recordRoute) );
         
         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }

   void testNoAliasRoutedWithAuthRecorded()
      {
         SipRouter sipRouter(*mUserAgent,
                             mForwardingRules,
                             true, // use auth proxy
                             "authproxy.example.com",
                             true  // add record route
                             );
         
         const char* message =
            "INVITE sip:user@external.example.net SIP/2.0\r\n"
            "Route: <sip:somewhere.net;lr>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@external.example.net\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         
         CPPUNIT_ASSERT(sipRouter.proxyMessage(testMsg));

         // UtlString proxiedMsg;
         // int msgLen;
         // testMsg.getBytes(&proxiedMsg, &msgLen);
         // printf("In:\n%s\nOut:\n%s\n", message, proxiedMsg.data());

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@external.example.net", requestUri.data());

         UtlString topRoute;
         CPPUNIT_ASSERT( testMsg.getRouteUri(0, &topRoute) );
         ASSERT_STR_EQUAL("<sip:authproxy.example.com;lr>", topRoute.data());
         
         UtlString nextRoute;
         CPPUNIT_ASSERT( testMsg.getRouteUri(1, &nextRoute) );
         ASSERT_STR_EQUAL("<sip:somewhere.net;lr>", nextRoute.data());
         
         UtlString noRoute;
         CPPUNIT_ASSERT( !testMsg.getRouteUri(2, &noRoute) );
         
         UtlString recordRoute;
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri(0, &recordRoute) );
         ASSERT_STR_EQUAL("<sip:127.0.0.2;lr>", recordRoute.data());
         
         UtlString noRecordRoute;
         CPPUNIT_ASSERT( ! testMsg.getRecordRouteUri(1, &recordRoute) );
         
         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }

};

const char* SipRouterTest::VoiceMail   = "Voicemail";
const char* SipRouterTest::MediaServer = "Mediaserver";
const char* SipRouterTest::LocalHost   = "localhost";
   
CPPUNIT_TEST_SUITE_REGISTRATION(SipRouterTest);
