//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
#include <utl/UtlHashMap.h>

#include <os/OsDefs.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>

/**
 * Unittest for SipMessage::normalizeProxyRoutes
 */
class SipProxyMessageTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipProxyMessageTest);
   CPPUNIT_TEST(testNoRouteLocal);
   CPPUNIT_TEST(testOneRouteLocal);
   CPPUNIT_TEST(testOneRouteNonLocal);
   CPPUNIT_TEST(testLocalRouteToOther);
   CPPUNIT_TEST(testTwoRoutesOneLocal);
   CPPUNIT_TEST(testLocalStrictRoute);
   CPPUNIT_TEST(testNonLocalStrictRoute);
   CPPUNIT_TEST(testDoubleRoute);
   CPPUNIT_TEST(testDoubleStrictRoute);
   CPPUNIT_TEST(testLotsaRoutes);
   CPPUNIT_TEST_SUITE_END();

private:

   SipUserAgent* UserAgent;

public:

   void setUp()
      {
         // Construct a SipUserAgent to provide the isMyHostAlias recognizer

         UserAgent = new SipUserAgent(0, 0, -1);

         UtlString internalDomainAlias("example.com:5060");
         UserAgent->setHostAliases(internalDomainAlias);

         UtlString internalHostAlias("internal.example.com:5060");
         UserAgent->setHostAliases(internalHostAlias);

         UtlString externalAlias("external.example.net:5060");
         UserAgent->setHostAliases(externalAlias);
      }

   void tearDown()
      {
         delete UserAgent;
      }

   bool getNormalRoute(const SipMessage& sipMsg, int routeIndex, UtlString& route)
      {
         UtlString rawRoute;
         bool gotRoute;

         gotRoute = sipMsg.getRouteUri(routeIndex, &rawRoute);
         if (gotRoute)
         {
            Url url(rawRoute);
            url.toString(route);
         }
         else
         {
            route.remove(0);
         }

         return gotRoute;
      }

   void testNoRouteLocal()
      {
         const char* message =
            "INVITE sip:user@example.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         Url requestUri;
         UtlSList removedRoutes;

         testMsg.normalizeProxyRoutes(UserAgent, requestUri, &removedRoutes);

         UtlString normalizedMsg;
         ssize_t msgLen;
         testMsg.getBytes(&normalizedMsg, &msgLen);

         ASSERT_STR_EQUAL(message, normalizedMsg.data());

         UtlString requestUriResult;
         requestUri.toString(requestUriResult);

         ASSERT_STR_EQUAL("sip:user@example.com", requestUriResult.data());

         CPPUNIT_ASSERT(removedRoutes.isEmpty());
      }

   void testOneRouteLocal()
      {
         const char* message =
            "INVITE sip:user@example.com SIP/2.0\r\n"
            "Route: <sip:example.com;lr>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         Url requestUri;
         UtlSList removedRoutes;

         testMsg.normalizeProxyRoutes(UserAgent, requestUri, &removedRoutes);

         UtlString normalizedMsg;
         ssize_t msgLen;
         testMsg.getBytes(&normalizedMsg, &msgLen);

         const char* expectedMessage = // route header removed
            "INVITE sip:user@example.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         ASSERT_STR_EQUAL(expectedMessage, normalizedMsg.data());

         UtlString requestUriResult;
         requestUri.toString(requestUriResult);

         ASSERT_STR_EQUAL("sip:user@example.com", requestUriResult.data());

         UtlString* removedRoute;
         CPPUNIT_ASSERT( removedRoute = dynamic_cast<UtlString*>(removedRoutes.get()));
         ASSERT_STR_EQUAL("<sip:example.com;lr>", removedRoute->data());
         delete removedRoute;

         CPPUNIT_ASSERT(removedRoutes.isEmpty());
      }

   void testOneRouteNonLocal()
      {
         const char* message =
            "INVITE sip:user@example.com SIP/2.0\r\n"
            "Route: <sip:other.example.edu;lr>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         Url requestUri;
         UtlSList removedRoutes;

         testMsg.normalizeProxyRoutes(UserAgent, requestUri, &removedRoutes);
         UtlString normalizedMsg;
         ssize_t msgLen;
         testMsg.getBytes(&normalizedMsg, &msgLen);

         ASSERT_STR_EQUAL(message, normalizedMsg.data());

         UtlString requestUriResult;
         requestUri.toString(requestUriResult);

         ASSERT_STR_EQUAL("sip:user@example.com", requestUriResult.data());

         CPPUNIT_ASSERT(removedRoutes.isEmpty());
      }

   void testLocalRouteToOther()
      {
         const char* message =
            "INVITE sip:user@other.example.edu SIP/2.0\r\n"
            "Route: <sip:internal.example.com;lr>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         Url requestUri;
         UtlSList removedRoutes;

         testMsg.normalizeProxyRoutes(UserAgent, requestUri, &removedRoutes);

         UtlString normalizedMsg;
         ssize_t msgLen;
         testMsg.getBytes(&normalizedMsg, &msgLen);

         const char* expectedMessage = // route header removed
            "INVITE sip:user@other.example.edu SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         ASSERT_STR_EQUAL(expectedMessage, normalizedMsg.data());

         UtlString requestUriResult;
         requestUri.toString(requestUriResult);

         ASSERT_STR_EQUAL("sip:user@other.example.edu", requestUriResult.data());

         UtlString* removedRoute;
         CPPUNIT_ASSERT( removedRoute = dynamic_cast<UtlString*>(removedRoutes.get()));
         ASSERT_STR_EQUAL("<sip:internal.example.com;lr>", removedRoute->data());
         delete removedRoute;

         CPPUNIT_ASSERT(removedRoutes.isEmpty());

      }

   void testTwoRoutesOneLocal()
      {
         const char* message =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Route: <sip:example.com;lr>, <sip:proxy.somewhere.com;lr>\r\n"
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
         Url requestUri;
         UtlSList removedRoutes;

         testMsg.normalizeProxyRoutes(UserAgent, requestUri, &removedRoutes);

         UtlString normalizedMsg;
         ssize_t msgLen;
         testMsg.getBytes(&normalizedMsg, &msgLen);

         UtlString requestUriResult;
         requestUri.toString(requestUriResult);

         ASSERT_STR_EQUAL("sip:user@somewhere.com", requestUriResult.data());

         UtlString* removedRoute;

         CPPUNIT_ASSERT( removedRoute = dynamic_cast<UtlString*>(removedRoutes.get()));
         ASSERT_STR_EQUAL("<sip:example.com;lr>", removedRoute->data());
         delete removedRoute;

         CPPUNIT_ASSERT(removedRoutes.isEmpty());

         UtlString route;
         /* "Route: <sip:proxy.somewhere.com;lr>\r\n" */

         CPPUNIT_ASSERT(getNormalRoute(testMsg, 0, route));
         ASSERT_STR_EQUAL("<sip:proxy.somewhere.com;lr>", route.data());

         CPPUNIT_ASSERT(!getNormalRoute(testMsg, 1, route));

      }

   void testLocalStrictRoute()
      {
         const char* message =
            "INVITE sip:example.com;lr SIP/2.0\r\n"
            "Route: <sip:user@example.com>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         Url requestUri;
         UtlSList removedRoutes;

         testMsg.normalizeProxyRoutes(UserAgent, requestUri, &removedRoutes);

         UtlString normalizedMsg;
         ssize_t msgLen;
         testMsg.getBytes(&normalizedMsg, &msgLen);

         const char* expectedMessage = // route header removed, uri swapped
            "INVITE sip:user@example.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         ASSERT_STR_EQUAL(expectedMessage, normalizedMsg.data());

         UtlString requestUriResult;
         requestUri.toString(requestUriResult);

         ASSERT_STR_EQUAL("sip:user@example.com", requestUriResult.data());
      }

   void testNonLocalStrictRoute()
      {
         const char* message =
            "INVITE sip:example.com;lr SIP/2.0\r\n"
            "Route: <sip:user@foreign.example.net;lr>, <sip:user@other.example.edu>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         Url requestUri;
         UtlSList removedRoutes;

         testMsg.normalizeProxyRoutes(UserAgent, requestUri, &removedRoutes);

         UtlString normalizedMsg;
         ssize_t msgLen;
         testMsg.getBytes(&normalizedMsg, &msgLen);

         const char* expectedMessage = // route header removed, uri swapped
            "INVITE sip:user@other.example.edu SIP/2.0\r\n"
            "Route: <sip:user@foreign.example.net;lr>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         ASSERT_STR_EQUAL(expectedMessage, normalizedMsg.data());

         UtlString requestUriResult;
         requestUri.toString(requestUriResult);

         ASSERT_STR_EQUAL("sip:user@other.example.edu", requestUriResult.data());

         UtlString* removedRoute;
         CPPUNIT_ASSERT( removedRoute = dynamic_cast<UtlString*>(removedRoutes.get()));
         ASSERT_STR_EQUAL("<sip:example.com;lr>", removedRoute->data());
         delete removedRoute;

         CPPUNIT_ASSERT(removedRoutes.isEmpty());
      }

   void testDoubleRoute()
      {
         const char* message =
            "INVITE sip:user@example.com SIP/2.0\r\n"
            "Route: <sip:user@internal.example.com;lr>, <sip:user@external.example.net;lr>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         Url requestUri;
         UtlSList removedRoutes;

         testMsg.normalizeProxyRoutes(UserAgent, requestUri, &removedRoutes);

         UtlString normalizedMsg;
         ssize_t msgLen;
         testMsg.getBytes(&normalizedMsg, &msgLen);

         const char* expectedMessage = // route header removed, uri swapped
            "INVITE sip:user@example.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         ASSERT_STR_EQUAL(expectedMessage, normalizedMsg.data());

         UtlString requestUriResult;
         requestUri.toString(requestUriResult);

         ASSERT_STR_EQUAL("sip:user@example.com", requestUriResult.data());

         UtlString* removedRoute;

         CPPUNIT_ASSERT( removedRoute = dynamic_cast<UtlString*>(removedRoutes.get()));
         ASSERT_STR_EQUAL("<sip:user@internal.example.com;lr>", removedRoute->data());
         delete removedRoute;

         CPPUNIT_ASSERT( removedRoute = dynamic_cast<UtlString*>(removedRoutes.get()));
         ASSERT_STR_EQUAL("<sip:user@external.example.net;lr>", removedRoute->data());
         delete removedRoute;

         CPPUNIT_ASSERT(removedRoutes.isEmpty());

      }

   void testDoubleStrictRoute()
      {
         const char* message =
            "INVITE sip:user@external.example.net;lr SIP/2.0\r\n"
            "Route: <sip:user@internal.example.com;lr>, <sip:user@example.com>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         Url requestUri;
         UtlSList removedRoutes;

         testMsg.normalizeProxyRoutes(UserAgent, requestUri, &removedRoutes);

         UtlString normalizedMsg;
         ssize_t msgLen;
         testMsg.getBytes(&normalizedMsg, &msgLen);

         const char* expectedMessage = // route header removed, uri swapped
            "INVITE sip:user@example.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         ASSERT_STR_EQUAL(expectedMessage, normalizedMsg.data());

         UtlString requestUriResult;
         requestUri.toString(requestUriResult);

         ASSERT_STR_EQUAL("sip:user@example.com", requestUriResult.data());

         UtlString* removedRoute;

         CPPUNIT_ASSERT( removedRoute = dynamic_cast<UtlString*>(removedRoutes.get()));
         ASSERT_STR_EQUAL("<sip:user@external.example.net;lr>", removedRoute->data());
         delete removedRoute;

         CPPUNIT_ASSERT( removedRoute = dynamic_cast<UtlString*>(removedRoutes.get()));
         ASSERT_STR_EQUAL("<sip:user@internal.example.com;lr>", removedRoute->data());
         delete removedRoute;

         CPPUNIT_ASSERT(removedRoutes.isEmpty());
      }

   void testLotsaRoutes()
      {
         const char* message =
            "INVITE sip:example.com;lr SIP/2.0\r\n"
            "Route: "
            "<sip:internal.example.com;lr>, "
            "<sip:external.example.net;lr>, "
            "<sip:somewhere.else.net;lr>, "
            "<sip:some.other.net;lr>, "
            "<sip:somewhereelse.org;lr>, "
            "<sip:some-other.net;lr>, "
            "<sip:proxy.other.net;lr>, "
            "<sip:user@foobar.com>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage testMsg(message, strlen(message));
         Url requestUri;
         UtlSList removedRoutes;

         testMsg.normalizeProxyRoutes(UserAgent, requestUri, &removedRoutes);

         UtlString normalizedMsg;
         ssize_t msgLen;
         testMsg.getBytes(&normalizedMsg, &msgLen);

         UtlString requestUriResult;
         requestUri.toString(requestUriResult);

         ASSERT_STR_EQUAL("sip:user@foobar.com", requestUriResult.data());

         UtlString* removedRoute;

         CPPUNIT_ASSERT( removedRoute = dynamic_cast<UtlString*>(removedRoutes.get()));
         ASSERT_STR_EQUAL("<sip:example.com;lr>", removedRoute->data());
         delete removedRoute;

         CPPUNIT_ASSERT( removedRoute = dynamic_cast<UtlString*>(removedRoutes.get()));
         ASSERT_STR_EQUAL("<sip:internal.example.com;lr>", removedRoute->data());
         delete removedRoute;

         CPPUNIT_ASSERT( removedRoute = dynamic_cast<UtlString*>(removedRoutes.get()));
         ASSERT_STR_EQUAL("<sip:external.example.net;lr>", removedRoute->data());
         delete removedRoute;

         CPPUNIT_ASSERT(removedRoutes.isEmpty());

         UtlString route;
         /* "Route: "
          * "<sip:somewhere.else.net;lr>, "
          * "<sip:some.other.net;lr>, "
          * "<sip:somewhereelse.org;lr>, "
          * "<sip:some-other.net;lr>, "
          * "<sip:proxy.other.net;lr>"
          */

         CPPUNIT_ASSERT(getNormalRoute(testMsg, 0, route));
         ASSERT_STR_EQUAL("<sip:somewhere.else.net;lr>", route.data());

         CPPUNIT_ASSERT(getNormalRoute(testMsg, 1, route));
         ASSERT_STR_EQUAL("<sip:some.other.net;lr>", route.data());

         CPPUNIT_ASSERT(getNormalRoute(testMsg, 2, route));
         ASSERT_STR_EQUAL("<sip:somewhereelse.org;lr>", route.data());

         CPPUNIT_ASSERT(getNormalRoute(testMsg, 3, route));
         ASSERT_STR_EQUAL("<sip:some-other.net;lr>", route.data());

         CPPUNIT_ASSERT(getNormalRoute(testMsg, 4, route));
         ASSERT_STR_EQUAL("<sip:proxy.other.net;lr>", route.data());

         CPPUNIT_ASSERT(!getNormalRoute(testMsg, 5, route));
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipProxyMessageTest);
