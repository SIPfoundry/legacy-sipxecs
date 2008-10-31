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
#include "testlib/SipDbTestContext.h"


#include <os/OsDefs.h>
#include <os/OsConfigDb.h>
#include <os/OsSysLog.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include "sipdb/CredentialDB.h"
#include "ForwardRules.h"
#include "SipRouter.h"
#include "CallerAlias.h"

extern CallerAlias* CallerAlias::spInstance;

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

   CPPUNIT_TEST(testSupportedOptions);
   CPPUNIT_TEST(testUnsupportedOptionAlone);
   CPPUNIT_TEST(testUnsupportedOptionFirst);
   CPPUNIT_TEST(testUnsupportedOptionMiddle);
   CPPUNIT_TEST(testUnsupportedOptionLast);
   CPPUNIT_TEST(testGenericProxy);
   CPPUNIT_TEST(testNoMaxForwards);
   CPPUNIT_TEST(testRecordRouteNoForwardingRuleMatch);
   CPPUNIT_TEST(testRecordRouteForwardingRuleMatch);
   CPPUNIT_TEST(testRecordRouteOrder);
   CPPUNIT_TEST(testAliasRoute);
   CPPUNIT_TEST(testNoAliasRouted);
   CPPUNIT_TEST(testProxySpiralingRequest);
   CPPUNIT_TEST(testProxyEndOfSpiralRequest);
   CPPUNIT_TEST(testInDialogRequestSelf);
   CPPUNIT_TEST(testInDialogRequestOther);
   CPPUNIT_TEST(testProxyChallengeLocal);
   
   CPPUNIT_TEST_SUITE_END();

private:

   SipUserAgent* mUserAgent;
   SipRouter*    mSipRouter;
   ForwardRules  mForwardingRules;

   static const char* VoiceMail;
   static const char* MediaServer;
   static const char* LocalHost;
   static const char* SipRouterConfiguration;
   
public:
   static SipDbTestContext TestDbContext;

   void setUp()
      {
         // prevent the test from using any installed configuration files
         TestDbContext.setSipxDir(SipXecsService::ConfigurationDirType);
         TestDbContext.setSipxDir(SipXecsService::DatabaseDirType);
         TestDbContext.inputFile("credential.xml");

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

         UtlString rulesFile;
         TestDbContext.inputFilePath("routing.xml", rulesFile);

         CPPUNIT_ASSERT(OS_SUCCESS ==
                        mForwardingRules.loadMappings(rulesFile,
                                                      MediaServer, VoiceMail, LocalHost));

         OsConfigDb testConfigDb;
         CPPUNIT_ASSERT( testConfigDb.loadFromBuffer( SipRouterConfiguration ) == OS_SUCCESS );
         
         CallerAlias::spInstance = 0;
         
         mSipRouter = new SipRouter(*mUserAgent,
                                     mForwardingRules,
                                     testConfigDb );
      }

   void tearDown()
      {
         delete mSipRouter;
         delete mUserAgent;

         CredentialDB::getInstance()->releaseInstance();
      }

   void testSupportedOptions()
      {
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
            "Proxy-Require: 100rel,replaces"
            "\r\n";
         mUserAgent->allowExtension("100rel");
         mUserAgent->allowExtension("replaces");
         SipMessage testMsg(message, strlen(message));
         SipMessage testRsp;
         CPPUNIT_ASSERT_EQUAL(SipRouter::SendRequest, mSipRouter->proxyMessage(testMsg, testRsp));
      }

   void testUnsupportedOptionAlone()
      {
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
            "Proxy-Require: DavesInsanitySauce"
            "\r\n";
         mUserAgent->allowExtension("100rel");
         mUserAgent->allowExtension("replaces");
         SipMessage testMsg(message, strlen(message));
         SipMessage testRsp;
         CPPUNIT_ASSERT_EQUAL(SipRouter::SendResponse,mSipRouter->proxyMessage(testMsg, testRsp));
         CPPUNIT_ASSERT_EQUAL(SIP_BAD_EXTENSION_CODE, testRsp.getResponseStatusCode());
         ASSERT_STR_EQUAL("DavesInsanitySauce",testRsp.getHeaderValue(0, SIP_UNSUPPORTED_FIELD));
         CPPUNIT_ASSERT(!testRsp.getHeaderValue(1, SIP_UNSUPPORTED_FIELD));
      }

   void testUnsupportedOptionFirst()
      {
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
            "Proxy-Require: DavesInsanitySauce,100rel,replaces"
            "\r\n";
         mUserAgent->allowExtension("100rel");
         mUserAgent->allowExtension("replaces");
         SipMessage testMsg(message, strlen(message));
         SipMessage testRsp;
         CPPUNIT_ASSERT_EQUAL(SipRouter::SendResponse,mSipRouter->proxyMessage(testMsg, testRsp));
         CPPUNIT_ASSERT_EQUAL(SIP_BAD_EXTENSION_CODE, testRsp.getResponseStatusCode());
         ASSERT_STR_EQUAL("DavesInsanitySauce",testRsp.getHeaderValue(0, SIP_UNSUPPORTED_FIELD));
         CPPUNIT_ASSERT(!testRsp.getHeaderValue(1, SIP_UNSUPPORTED_FIELD));
      }

   void testUnsupportedOptionMiddle()
      {
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
            "Proxy-Require: 100rel,DavesInsanitySauce,replaces"
            "\r\n";
         mUserAgent->allowExtension("100rel");
         mUserAgent->allowExtension("replaces");
         SipMessage testMsg(message, strlen(message));
         SipMessage testRsp;
         CPPUNIT_ASSERT_EQUAL(SipRouter::SendResponse,mSipRouter->proxyMessage(testMsg, testRsp));
         CPPUNIT_ASSERT_EQUAL(SIP_BAD_EXTENSION_CODE, testRsp.getResponseStatusCode());
         ASSERT_STR_EQUAL("DavesInsanitySauce",testRsp.getHeaderValue(0, SIP_UNSUPPORTED_FIELD));
         CPPUNIT_ASSERT(!testRsp.getHeaderValue(1, SIP_UNSUPPORTED_FIELD));
      }

   void testUnsupportedOptionLast()
      {
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
            "Proxy-Require: 100rel,replaces,DavesInsanitySauce"
            "\r\n";
         mUserAgent->allowExtension("100rel");
         mUserAgent->allowExtension("replaces");
         SipMessage testMsg(message, strlen(message));
         SipMessage testRsp;
         CPPUNIT_ASSERT_EQUAL(SipRouter::SendResponse,mSipRouter->proxyMessage(testMsg, testRsp));
         CPPUNIT_ASSERT_EQUAL(SIP_BAD_EXTENSION_CODE, testRsp.getResponseStatusCode());
         ASSERT_STR_EQUAL("DavesInsanitySauce",testRsp.getHeaderValue(0, SIP_UNSUPPORTED_FIELD));
         CPPUNIT_ASSERT(!testRsp.getHeaderValue(1, SIP_UNSUPPORTED_FIELD));
      }
   
   void testGenericProxy()
      {
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
         SipMessage testRsp;
         
         CPPUNIT_ASSERT_EQUAL(SipRouter::SendRequest,mSipRouter->proxyMessage(testMsg, testRsp));

         UtlString proxiedMsg;
         ssize_t msgLen;
         testMsg.getBytes(&proxiedMsg, &msgLen);

         OsSysLog::add(FAC_SIP, PRI_INFO, "Proxied Message:\n%s", proxiedMsg.data());
         
         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@somewhere.com", requestUri.data());
         
         UtlString noRoute;
         CPPUNIT_ASSERT( !testMsg.getRouteUri(0, &noRoute) );
         
         // SipRouter always Record-Route requests
         UtlString recordRoute;
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri(0, &recordRoute) );
         
         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }

   void testNoMaxForwards()
      {
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
         SipMessage testRsp;
         
         CPPUNIT_ASSERT_EQUAL(SipRouter::SendRequest,mSipRouter->proxyMessage(testMsg, testRsp));

         UtlString proxiedMsg;
         ssize_t msgLen;
         testMsg.getBytes(&proxiedMsg, &msgLen);

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@somewhere.com", requestUri.data());
         
         UtlString noRoute;
         CPPUNIT_ASSERT( !testMsg.getRouteUri(0, &noRoute) );
         
         UtlString recordRoute;
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri(0, &recordRoute) );
         
         int maxForwards;
         CPPUNIT_ASSERT(   testMsg.getMaxForwards(maxForwards)
                        && maxForwards == SIP_DEFAULT_MAX_FORWARDS);
      }   

   
   void testRecordRouteNoForwardingRuleMatch()
      {
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
         SipMessage testRsp;
         
         CPPUNIT_ASSERT_EQUAL(SipRouter::SendRequest,mSipRouter->proxyMessage(testMsg, testRsp));

         UtlString proxiedMsg;
         ssize_t msgLen;
         testMsg.getBytes(&proxiedMsg, &msgLen);

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@somewhere.com", requestUri.data());
         
         UtlString topRoute, recordRoute, tempString, urlParmName;
         CPPUNIT_ASSERT( ! testMsg.getRouteUri(0, &topRoute) );
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri(0, &recordRoute) );
         Url recordRouteUrl(recordRoute);
         recordRouteUrl.getUrlType( tempString );
         ASSERT_STR_EQUAL("sip", tempString.data());
         recordRouteUrl.getHostWithPort( tempString );
         ASSERT_STR_EQUAL("10.10.10.1:5060", tempString.data());
         CPPUNIT_ASSERT( recordRouteUrl.getUrlParameter( 0, urlParmName, tempString ) );
         ASSERT_STR_EQUAL("lr", urlParmName.data());
         CPPUNIT_ASSERT( recordRouteUrl.getUrlParameter( 1, urlParmName, tempString ) );
         ASSERT_STR_EQUAL("sipXecs-rs", urlParmName.data());

         CPPUNIT_ASSERT( !testMsg.getHeaderValue( 0, "X-SipX-Spiral" ) );

         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }   

   void testRecordRouteForwardingRuleMatch()
      {
         const char* message =
            "INVITE sip:user@internal.example.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@internal.example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         SipMessage testRsp;
         
         CPPUNIT_ASSERT_EQUAL(SipRouter::SendRequest,mSipRouter->proxyMessage(testMsg, testRsp));

         UtlString proxiedMsg;
         ssize_t msgLen;
         testMsg.getBytes(&proxiedMsg, &msgLen);

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@internal.example.com", requestUri.data());
         
         UtlString topRoute, recordRoute, tempString, urlParmName;
         CPPUNIT_ASSERT( testMsg.getRouteUri(0, &topRoute) );
         ASSERT_STR_EQUAL("<sip:registrar.example.com;lr>", topRoute.data());
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri(0, &recordRoute) );
         ASSERT_STR_EQUAL("<sip:10.10.10.1:5060;lr>", recordRoute.data());
         CPPUNIT_ASSERT( testMsg.getHeaderValue( 0, "X-SipX-Spiral" ) );
        
         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }   

   void testRecordRouteOrder()
      {
         const char* message =
            "INVITE sip:user@internal.example.com SIP/2.0\r\n"
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
         SipMessage testRsp;
         
         CPPUNIT_ASSERT_EQUAL(SipRouter::SendRequest,mSipRouter->proxyMessage(testMsg, testRsp));

         UtlString proxiedMsg;
         ssize_t msgLen;
         testMsg.getBytes(&proxiedMsg, &msgLen);

         UtlString requestUri;
         testMsg.getRequestUri(&requestUri);
         ASSERT_STR_EQUAL("sip:user@internal.example.com", requestUri.data());
         
         UtlString topRoute;
         CPPUNIT_ASSERT( testMsg.getRouteUri(0, &topRoute) );
         ASSERT_STR_EQUAL("<sip:registrar.example.com;lr>", topRoute.data());
         
         // Record-Route: <sip:10.10.10.1:5060;lr>, <sip:first.example.net;lr>

         UtlString myRecordRoute;
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri(0, &myRecordRoute) );
         ASSERT_STR_EQUAL("<sip:10.10.10.1:5060;lr>", myRecordRoute.data());
         
         UtlString existingRecordRoute;
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri(1, &existingRecordRoute) );
         ASSERT_STR_EQUAL("<sip:first.example.net;lr>", existingRecordRoute.data());
         
         UtlString nomoreRecordRoute;
         CPPUNIT_ASSERT( ! testMsg.getRecordRouteUri(2, &nomoreRecordRoute) );
        
         CPPUNIT_ASSERT( testMsg.getHeaderValue( 0, "X-SipX-Spiral" ) );

         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }   

   void testAliasRoute()
      {
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
         SipMessage testRsp;
         
         CPPUNIT_ASSERT_EQUAL(SipRouter::SendRequest,mSipRouter->proxyMessage(testMsg, testRsp));

         // UtlString proxiedMsg;
         // ssize_t msgLen;
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
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri(0, &recordRoute) );
         ASSERT_STR_EQUAL("<sip:10.10.10.1:5060;lr>", recordRoute.data());
         
         // Check that X-SipX-Spiral is not leaking.
         CPPUNIT_ASSERT( !testMsg.getHeaderValue( 0, "X-SipX-Spiral" ) );

         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }

   void testNoAliasRouted()
      {
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
         SipMessage testRsp;
         
         CPPUNIT_ASSERT_EQUAL(SipRouter::SendRequest,mSipRouter->proxyMessage(testMsg, testRsp));

         // UtlString proxiedMsg;
         // ssize_t msgLen;
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
         
         // Presence of Route header causes forwarding rules to be skipped 
         // and to be authorized right away
         UtlString recordRoute, tempString, urlParmName;
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri(0, &recordRoute) );
         Url recordRouteUrl(recordRoute);
         recordRouteUrl.getUrlType( tempString );
         ASSERT_STR_EQUAL("sip", tempString.data());
         recordRouteUrl.getHostWithPort( tempString );
         ASSERT_STR_EQUAL("10.10.10.1:5060", tempString.data());
         CPPUNIT_ASSERT( recordRouteUrl.getUrlParameter( 0, urlParmName, tempString ) );
         ASSERT_STR_EQUAL("lr", urlParmName.data());
         CPPUNIT_ASSERT( recordRouteUrl.getUrlParameter( 1, urlParmName, tempString ) );
         ASSERT_STR_EQUAL("sipXecs-rs", urlParmName.data());
         
         CPPUNIT_ASSERT( !testMsg.getHeaderValue( 0, "X-SipX-Spiral" ) );

         int maxForwards;
         CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
      }

   void testProxySpiralingRequest()
   {
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
          "X-SipX-Spiral: true"
          "\r\n";

       SipMessage testMsg(message, strlen(message));
       SipMessage testRsp;
       
       CPPUNIT_ASSERT_EQUAL(SipRouter::SendRequest,mSipRouter->proxyMessage(testMsg, testRsp));

       // UtlString proxiedMsg;
       // ssize_t msgLen;
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
       
       // verify the presence of a Record-Route to us containing a Route State
       UtlString recordRoute, tempString, urlParmName;
       CPPUNIT_ASSERT( testMsg.getRecordRouteUri(0, &recordRoute) );
       Url recordRouteUrl(recordRoute);
       recordRouteUrl.getUrlType( tempString );
       ASSERT_STR_EQUAL("sip", tempString.data());
       recordRouteUrl.getHostWithPort( tempString );
       ASSERT_STR_EQUAL("10.10.10.1:5060", tempString.data());
       CPPUNIT_ASSERT( recordRouteUrl.getUrlParameter( 0, urlParmName, tempString ) );
       ASSERT_STR_EQUAL("lr", urlParmName.data());
       CPPUNIT_ASSERT( recordRouteUrl.getUrlParameter( 1, urlParmName, tempString ) );
       ASSERT_STR_EQUAL("sipXecs-rs", urlParmName.data());
       
       // Check that X-SipX-Spiral is not leaking.
       CPPUNIT_ASSERT( !testMsg.getHeaderValue( 0, "X-SipX-Spiral" ) );

       int maxForwards;
       CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
    }

   void testProxyEndOfSpiralRequest()
    {
      const char* message =
         "INVITE sip:user@10.10.10.2 SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:user@external.example.net\r\n"
         "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 20\r\n"
         "Contact: caller@127.0.0.1\r\n"
         "Content-Length: 0\r\n"
         "X-SipX-Spiral: true"
         "\r\n";

      SipMessage testMsg(message, strlen(message));
      SipMessage testRsp;
      
      CPPUNIT_ASSERT_EQUAL(SipRouter::SendRequest,mSipRouter->proxyMessage(testMsg, testRsp));

      // UtlString proxiedMsg;
      // ssize_t msgLen;
      // testMsg.getBytes(&proxiedMsg, &msgLen);
      // printf("In:\n%s\nOut:\n%s\n", message, proxiedMsg.data());

      UtlString requestUri;
      testMsg.getRequestUri(&requestUri);
      ASSERT_STR_EQUAL("sip:user@10.10.10.2", requestUri.data());

      UtlString noRoute;
      CPPUNIT_ASSERT( !testMsg.getRouteUri(0, &noRoute) );
      
      // verify the presence of a Record-Route to us containing a Route State
      UtlString recordRoute, tempString, urlParmName;
      CPPUNIT_ASSERT( testMsg.getRecordRouteUri(0, &recordRoute) );
      Url recordRouteUrl(recordRoute);
      recordRouteUrl.getUrlType( tempString );
      ASSERT_STR_EQUAL("sip", tempString.data());
      recordRouteUrl.getHostWithPort( tempString );
      ASSERT_STR_EQUAL("10.10.10.1:5060", tempString.data());
      CPPUNIT_ASSERT( recordRouteUrl.getUrlParameter( 0, urlParmName, tempString ) );
      ASSERT_STR_EQUAL("lr", urlParmName.data());
      CPPUNIT_ASSERT( recordRouteUrl.getUrlParameter( 1, urlParmName, tempString ) );
      ASSERT_STR_EQUAL("sipXecs-rs", urlParmName.data());
      
      CPPUNIT_ASSERT( !testMsg.getHeaderValue( 0, "X-SipX-Spiral" ) );

      int maxForwards;
      CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
    }
   
   void testInDialogRequestSelf()
    {
      RouteState::setSecret("GuessThat!");

      const char* message =
         "INVITE sip:user@external.example.net SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:user@external.example.net\r\n"
         "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 INVITE\r\n"
         "Route: <sip:10.10.10.1:5060;lr;sipXecs-rs=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60%21607dcb78ef2addd25638653db3070349>\r\n"
         "Max-Forwards: 20\r\n"
         "Contact: caller@127.0.0.1\r\n"
         "Content-Length: 0\r\n"
         "\r\n";

      SipMessage testMsg(message, strlen(message));
      SipMessage testRsp;
      
      CPPUNIT_ASSERT_EQUAL(SipRouter::SendRequest,mSipRouter->proxyMessage(testMsg, testRsp));

      // UtlString proxiedMsg;
      // ssize_t msgLen;
      // testMsg.getBytes(&proxiedMsg, &msgLen);
      // printf("In:\n%s\nOut:\n%s\n", message, proxiedMsg.data());

      UtlString requestUri;
      testMsg.getRequestUri(&requestUri);
      ASSERT_STR_EQUAL("sip:user@external.example.net", requestUri.data());

      // verify that route has NOT been popped off.
      UtlString topRoute, tempString, urlParmName;
      CPPUNIT_ASSERT( testMsg.getRouteUri(0, &topRoute) );

      // verify that no new Record-Route was added
      UtlString recordRoute;
      CPPUNIT_ASSERT( !testMsg.getRecordRouteUri(0, &recordRoute) );
      
      CPPUNIT_ASSERT( !testMsg.getHeaderValue( 0, "X-SipX-Spiral" ) );

      int maxForwards;
      CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
    }


   void testInDialogRequestOther()
    {
      RouteState::setSecret("GuessThat!");

      const char* message =
         "INVITE sip:user@external.Notexample.net SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:user@external.Notexample.net\r\n"
         "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 INVITE\r\n"
         "Route: <sip:10.10.10.1:5060;lr;sipXecs-rs=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60%21607dcb78ef2addd25638653db3070349>\r\n"
         "Max-Forwards: 20\r\n"
         "Contact: caller@127.0.0.1\r\n"
         "Content-Length: 0\r\n"
         "\r\n";

      SipMessage testMsg(message, strlen(message));
      SipMessage testRsp;
      
      CPPUNIT_ASSERT_EQUAL(SipRouter::SendRequest,mSipRouter->proxyMessage(testMsg, testRsp));
      // UtlString proxiedMsg;
      // ssize_t msgLen;
      // testMsg.getBytes(&proxiedMsg, &msgLen);
      // printf("In:\n%s\nOut:\n%s\n", message, proxiedMsg.data());

      UtlString requestUri;
      testMsg.getRequestUri(&requestUri);
      ASSERT_STR_EQUAL("sip:user@external.Notexample.net", requestUri.data());

      // verify that route has been popped off.
      UtlString topRoute, tempString, urlParmName;
      CPPUNIT_ASSERT( !testMsg.getRouteUri(0, &topRoute) );

      // verify that no new Record-Route was added
      UtlString recordRoute;
      CPPUNIT_ASSERT( !testMsg.getRecordRouteUri(0, &recordRoute) );
      
      CPPUNIT_ASSERT( !testMsg.getHeaderValue( 0, "X-SipX-Spiral" ) );

      int maxForwards;
      CPPUNIT_ASSERT(testMsg.getMaxForwards(maxForwards) && maxForwards == 19);
    }

   void testProxyChallengeLocal()
      {
         const char* message =
            "INVITE sip:user@external.example.net SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Mighty Hunter <sip:mightyhunter@example.com>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: mightyhunter@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         SipMessage testRsp;
       
         CPPUNIT_ASSERT_EQUAL(SipRouter::SendResponse,mSipRouter->proxyMessage(testMsg, testRsp));

         ssize_t msgSize;
         UtlString challenge;
         testRsp.getBytes(&challenge, &msgSize);
         OsSysLog::add(FAC_SIP, PRI_INFO, "Returned Challenge:\n%s", challenge.data());

         CPPUNIT_ASSERT_EQUAL(HTTP_PROXY_UNAUTHORIZED_CODE, testRsp.getResponseStatusCode());
      }

};

const char* SipRouterTest::VoiceMail   = "Voicemail";
const char* SipRouterTest::MediaServer = "Mediaserver";
const char* SipRouterTest::LocalHost   = "localhost";
const char* SipRouterTest::SipRouterConfiguration =
   "SIPX_PROXY_AUTHENTICATE_REALM : example.com\r\n"
   "SIPX_PROXY_HOSTPORT : 10.10.10.1:5060\r\n"
   "SIPX_PROXY_DOMAIN_NAME : example.com\r\n"
   "SIPX_PROXY_HOOK_LIBRARY.authrules : ../../lib/authplugins/authplugins/.libs/libEnforceAuthRules.so\r\n"
   "SIPX_PROXY.authrules.IDENTITY_VALIDITY_SECONDS : 300\r\n"
   "SIPX_PROXY_HOOK_LIBRARY.fromalias : ../../lib/authplugins/authplugins/.libs/libCallerAlias.so\r\n"
   "\r\n";

CPPUNIT_TEST_SUITE_REGISTRATION(SipRouterTest);

SipDbTestContext  SipRouterTest::TestDbContext(TEST_DATA_DIR "/siproutertestdata", TEST_WORK_DIR "/siproutertestdata");

