//
// Copyright (C) 2008, 2010 Avaya, Inc., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <net/HttpMessage.h>
#include <net/NameValuePair.h>
#include <net/SipMessage.h>

class GetDigestAuthorizationDataTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(GetDigestAuthorizationDataTest);
    CPPUNIT_TEST(testMissingValues);
    CPPUNIT_TEST(testProxyNoHeader);
    CPPUNIT_TEST(testProxyOneHeader);
    CPPUNIT_TEST(testProxyTwoHeaders);
    CPPUNIT_TEST(testProxyOneHeaderWithAuthorizationHeader);
    CPPUNIT_TEST(testServerNoHeader);
    CPPUNIT_TEST(testServerOneHeader);
    CPPUNIT_TEST(testServerTwoHeaders);
    CPPUNIT_TEST(testServerOneHeaderWithAuthorizationHeader);
    CPPUNIT_TEST(testServerWithInstrument);
    CPPUNIT_TEST_SUITE_END();

   #define AUTHVALUE1 "Digest username=\"111\", realm=\"example.com\", nonce=\"606a7e9c58258179f966b0987a1bf38d1114527548\", uri=\"sip:111@example.com\", response=\"feaa478e10ee7d3ef6037746696bace6\", opaque=\"change4\""
   #define AUTHVALUE2 "Digest username=\"222\", realm=\"example.com\", nonce=\"606a7e9c58258279f966b0987a2bf38d2224527548\", uri=\"sip:222@example.com\", response=\"feaa478e20ee7d3ef6037746696bace6\", opaque=\"change4\""
   // username contains instrument indicator.
   #define AUTHVALUE3 "Digest username=\"111/foobar\", realm=\"example.com\", nonce=\"606a7e9c58258179f966b0987a1bf38d1114527548\", uri=\"sip:111@example.com\", response=\"feaa478e10ee7d3ef6037746696bace6\", opaque=\"change4\""
   // base user name contains '/'.
   #define AUTHVALUE4 "Digest username=\"Joe/111/foobar\", realm=\"example.com\", nonce=\"606a7e9c58258179f966b0987a1bf38d1114527548\", uri=\"sip:111@example.com\", response=\"feaa478e10ee7d3ef6037746696bace6\", opaque=\"change4\""

   UtlBoolean result;
   UtlString user;
   UtlString realm;
   UtlString nonce;
   UtlString opaque;
   UtlString response;
   UtlString uri;
   UtlString instrument;

public:
    void testMissingValues()
      {
         // Test that missing values in a header do not prevent
         // getDigestAuthorizationData from returning true (because callers'
         // loops test on that to determine whether to continue).
         const char* msg =
            "INVITE sip:14@example.com SIP/2.0\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length : 0\r\n"
            // Incorrect credential type.
            "Proxy-Authorization: Basic username=\"111\", realm=\"example.com\", nonce=\"606a7e9c58258179f966b0987a1bf38d1114527548\", uri=\"sip:111@example.com\", response=\"feaa478e10ee7d3ef6037746696bace6\", opaque=\"change4\"\r\n"
            // Missing response
            "Proxy-Authorization: Digest username=\"111\", realm=\"example.com\", nonce=\"606a7e9c58258179f966b0987a1bf38d1114527548\", uri=\"sip:111@example.com\", opaque=\"change4\"\r\n"
            // Missing username
            "Proxy-Authorization: Digest realm=\"example.com\", nonce=\"606a7e9c58258179f966b0987a1bf38d1114527548\", uri=\"sip:111@example.com\", response=\"feaa478e10ee7d3ef6037746696bace6\", opaque=\"change4\"\r\n"
            // Missing digest-uri
            "Proxy-Authorization: Digest username=\"111\", realm=\"example.com\", nonce=\"606a7e9c58258179f966b0987a1bf38d1114527548\", response=\"feaa478e10ee7d3ef6037746696bace6\", opaque=\"change4\"\r\n"
            // Missing realm
            "Proxy-Authorization: Digest username=\"111\", nonce=\"606a7e9c58258179f966b0987a1bf38d1114527548\", uri=\"sip:111@example.com\", response=\"feaa478e10ee7d3ef6037746696bace6\", opaque=\"change4\"\r\n"
            // Missing nonce
            "Proxy-Authorization: Digest username=\"111\", realm=\"example.com\", uri=\"sip:111@example.com\", response=\"feaa478e10ee7d3ef6037746696bace6\", opaque=\"change4\"\r\n"
            // Valid Proxy-Authorization
            "Proxy-Authorization: " AUTHVALUE2 "\r\n"
            "\r\n";
         HttpMessage *message = new HttpMessage(msg);

         // There are 6 unusable Proxy-Authorization headers.
         for (int i = 0; i < 6; i++)
         {
            result = message->getDigestAuthorizationData(&user,
                                                         &realm,
                                                         &nonce,
                                                         &opaque,
                                                         &response,
                                                         &uri,
                                                         NULL,  // TBD cnonce
                                                         NULL,  // TBD nonceCount
                                                         NULL,  // TBD qop
                                                         HttpMessage::PROXY,
                                                         0);
            CPPUNIT_ASSERT_MESSAGE("return should be true", true == result);
         }
         // Check the 7th header.
         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::PROXY,
                                                      6);
         CPPUNIT_ASSERT_MESSAGE("return should be true", true == result);
         ASSERT_STR_EQUAL_MESSAGE("wrong value for user", user.data(), "222");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for realm", realm.data(), "example.com");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for nonce", nonce.data(), "606a7e9c58258279f966b0987a2bf38d2224527548");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for opaque", opaque.data(), "change4");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for response", response.data(), "feaa478e20ee7d3ef6037746696bace6");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for uri", uri.data(), "sip:222@example.com");
         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::PROXY,
                                                      7);
         CPPUNIT_ASSERT_MESSAGE("return should be false", false == result);
      }
    void testProxyNoHeader()
      {
         const char* msg =
            "INVITE sip:14@example.com SIP/2.0\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length : 0\r\n"
            "\r\n";
         HttpMessage *message = new HttpMessage(msg);

         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::PROXY,
                                                      0);
         CPPUNIT_ASSERT_MESSAGE("return should be false", false == result);
      }
   void testProxyOneHeader()
      {
         const char* msg =
            "INVITE sip:14@example.com SIP/2.0\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length : 0\r\n"
            "Proxy-Authorization: " AUTHVALUE1 "\r\n"
            "\r\n";
         HttpMessage *message = new HttpMessage(msg);

         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::PROXY,
                                                      0);
         CPPUNIT_ASSERT_MESSAGE("return should be true", true == result);
         ASSERT_STR_EQUAL_MESSAGE("wrong value for user", user.data(), "111");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for realm", realm.data(), "example.com");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for nonce", nonce.data(), "606a7e9c58258179f966b0987a1bf38d1114527548");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for opaque", opaque.data(), "change4");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for response", response.data(), "feaa478e10ee7d3ef6037746696bace6");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for uri", uri.data(), "sip:111@example.com");
         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::PROXY,
                                                      1);
         CPPUNIT_ASSERT_MESSAGE("return should be false", false == result);
      }
   void testProxyTwoHeaders()
      {
         const char* msg =
            "INVITE sip:14@example.com SIP/2.0\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length : 0\r\n"
            "Proxy-Authorization: " AUTHVALUE1 "\r\n"
            "Proxy-Authorization: " AUTHVALUE2 "\r\n"
            "\r\n";
         HttpMessage *message = new HttpMessage(msg);

         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::PROXY,
                                                      0);
         CPPUNIT_ASSERT_MESSAGE("return should be true", true == result);
         ASSERT_STR_EQUAL_MESSAGE("wrong value for user", user.data(), "111");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for realm", realm.data(), "example.com");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for nonce", nonce.data(), "606a7e9c58258179f966b0987a1bf38d1114527548");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for opaque", opaque.data(), "change4");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for response", response.data(), "feaa478e10ee7d3ef6037746696bace6");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for uri", uri.data(), "sip:111@example.com");
         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::PROXY,
                                                      1);
         CPPUNIT_ASSERT_MESSAGE("return should be true", true == result);
         ASSERT_STR_EQUAL_MESSAGE("wrong value for user", user.data(), "222");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for realm", realm.data(), "example.com");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for nonce", nonce.data(), "606a7e9c58258279f966b0987a2bf38d2224527548");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for opaque", opaque.data(), "change4");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for response", response.data(), "feaa478e20ee7d3ef6037746696bace6");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for uri", uri.data(), "sip:222@example.com");
         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::PROXY,
                                                      2);
         CPPUNIT_ASSERT_MESSAGE("return should be false", false == result);
      }
   void testProxyOneHeaderWithAuthorizationHeader()
      {
         const char* msg =
            "INVITE sip:14@example.com SIP/2.0\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length : 0\r\n"
            "Authorization: " AUTHVALUE2 "\r\n"
            "Proxy-Authorization: " AUTHVALUE1 "\r\n"
            "\r\n";
         HttpMessage *message = new HttpMessage(msg);

         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::PROXY,
                                                      0);
         CPPUNIT_ASSERT_MESSAGE("return should be true", true == result);
         ASSERT_STR_EQUAL_MESSAGE("wrong value for user", user.data(), "111");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for realm", realm.data(), "example.com");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for nonce", nonce.data(), "606a7e9c58258179f966b0987a1bf38d1114527548");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for opaque", opaque.data(), "change4");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for response", response.data(), "feaa478e10ee7d3ef6037746696bace6");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for uri", uri.data(), "sip:111@example.com");
         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::PROXY,
                                                      1);
         CPPUNIT_ASSERT_MESSAGE("return should be false", false == result);
      }
    void testServerNoHeader()
      {
         const char* msg =
            "INVITE sip:14@example.com SIP/2.0\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length : 0\r\n"
            "\r\n";
         HttpMessage *message = new HttpMessage(msg);

         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::SERVER,
                                                      0);
         CPPUNIT_ASSERT_MESSAGE("return should be false", false == result);
      }
   void testServerOneHeader()
      {
         const char* msg =
            "INVITE sip:14@example.com SIP/2.0\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length : 0\r\n"
            "Authorization: " AUTHVALUE1 "\r\n"
            "\r\n";
         HttpMessage *message = new HttpMessage(msg);

         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::SERVER,
                                                      0);
         CPPUNIT_ASSERT_MESSAGE("return should be true", true == result);
         ASSERT_STR_EQUAL_MESSAGE("wrong value for user", user.data(), "111");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for realm", realm.data(), "example.com");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for nonce", nonce.data(), "606a7e9c58258179f966b0987a1bf38d1114527548");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for opaque", opaque.data(), "change4");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for response", response.data(), "feaa478e10ee7d3ef6037746696bace6");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for uri", uri.data(), "sip:111@example.com");
         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::SERVER,
                                                      1);
         CPPUNIT_ASSERT_MESSAGE("return should be false", false == result);
      }
   void testServerTwoHeaders()
      {
         const char* msg =
            "INVITE sip:14@example.com SIP/2.0\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length : 0\r\n"
            "Authorization: " AUTHVALUE1 "\r\n"
            "Authorization: " AUTHVALUE2 "\r\n"
            "\r\n";
         HttpMessage *message = new HttpMessage(msg);

         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::SERVER,
                                                      0);
         CPPUNIT_ASSERT_MESSAGE("return should be true", true == result);
         ASSERT_STR_EQUAL_MESSAGE("wrong value for user", user.data(), "111");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for realm", realm.data(), "example.com");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for nonce", nonce.data(), "606a7e9c58258179f966b0987a1bf38d1114527548");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for opaque", opaque.data(), "change4");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for response", response.data(), "feaa478e10ee7d3ef6037746696bace6");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for uri", uri.data(), "sip:111@example.com");
         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::SERVER,
                                                      1);
         CPPUNIT_ASSERT_MESSAGE("return should be true", true == result);
         ASSERT_STR_EQUAL_MESSAGE("wrong value for user", user.data(), "222");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for realm", realm.data(), "example.com");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for nonce", nonce.data(), "606a7e9c58258279f966b0987a2bf38d2224527548");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for opaque", opaque.data(), "change4");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for response", response.data(), "feaa478e20ee7d3ef6037746696bace6");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for uri", uri.data(), "sip:222@example.com");
         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::SERVER,
                                                      2);
         CPPUNIT_ASSERT_MESSAGE("return should be false", false == result);
      }
   void testServerOneHeaderWithAuthorizationHeader()
      {
         const char* msg =
            "INVITE sip:14@example.com SIP/2.0\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length : 0\r\n"
            "Proxy-Authorization: " AUTHVALUE2 "\r\n"
            "Authorization: " AUTHVALUE1 "\r\n"
            "\r\n";
         HttpMessage *message = new HttpMessage(msg);

         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::SERVER,
                                                      0);
         CPPUNIT_ASSERT_MESSAGE("return should be true", true == result);
         ASSERT_STR_EQUAL_MESSAGE("wrong value for user", user.data(), "111");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for realm", realm.data(), "example.com");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for nonce", nonce.data(), "606a7e9c58258179f966b0987a1bf38d1114527548");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for opaque", opaque.data(), "change4");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for response", response.data(), "feaa478e10ee7d3ef6037746696bace6");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for uri", uri.data(), "sip:111@example.com");
         result = message->getDigestAuthorizationData(&user,
                                                      &realm,
                                                      &nonce,
                                                      &opaque,
                                                      &response,
                                                      &uri,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::SERVER,
                                                      1);
         CPPUNIT_ASSERT_MESSAGE("return should be false", false == result);
      }
   void testServerWithInstrument()
      {
         const char* msg;
         HttpMessage *message;
         UtlString user;
         UtlString user_base;
         UtlString instrument;

         // Instrument value is not present.
         msg = 
            "INVITE sip:14@example.com SIP/2.0\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length : 0\r\n"
            "Authorization: " AUTHVALUE1 "\r\n"
            "\r\n";
         message = new HttpMessage(msg);
         result = message->getDigestAuthorizationData(&user,
                                                      NULL, 
                                                      NULL, 
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::SERVER,
                                                      0,
                                                      &user_base,
                                                      &instrument);
         ASSERT_STR_EQUAL_MESSAGE("wrong value for user_base", user_base.data(), "111");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for instrument", instrument.data(), "");
         
         // Instrument value is present.
         msg = 
            "INVITE sip:14@example.com SIP/2.0\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length : 0\r\n"
            "Authorization: " AUTHVALUE3 "\r\n"
            "\r\n";
         message = new HttpMessage(msg);
         result = message->getDigestAuthorizationData(&user,
                                                      NULL, 
                                                      NULL, 
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::SERVER,
                                                      0,
                                                      &user_base,
                                                      &instrument);
         ASSERT_STR_EQUAL_MESSAGE("wrong value for user_base", user_base.data(), "111");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for instrument", instrument.data(), "foobar");

         // User base value contains '/'.
         msg = 
            "INVITE sip:14@example.com SIP/2.0\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length : 0\r\n"
            "Authorization: " AUTHVALUE4 "\r\n"
            "\r\n";
         message = new HttpMessage(msg);
         result = message->getDigestAuthorizationData(&user,
                                                      NULL, 
                                                      NULL, 
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,  // TBD cnonce
                                                      NULL,  // TBD nonceCount
                                                      NULL,  // TBD qop
                                                      HttpMessage::SERVER,
                                                      0,
                                                      &user_base,
                                                      &instrument);
         ASSERT_STR_EQUAL_MESSAGE("wrong value for user_base", user_base.data(), "Joe/111");
         ASSERT_STR_EQUAL_MESSAGE("wrong value for instrument", instrument.data(), "foobar");
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(GetDigestAuthorizationDataTest);
