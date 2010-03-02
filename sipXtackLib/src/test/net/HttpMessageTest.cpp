//
// Copyright (C) 2007, 2010 Avaya, Inc., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <net/HttpMessage.h>
#include <net/SdpBody.h>


/**
 * Unittest for HttpMessage
 */
class HttpMessageTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(HttpMessageTest);
    CPPUNIT_TEST(testCreator);
    CPPUNIT_TEST(testMessage);
    CPPUNIT_TEST(testHeader);
    CPPUNIT_TEST(testSdp);
    CPPUNIT_TEST(testMd5Digest);
    CPPUNIT_TEST(testEscape);
    CPPUNIT_TEST(testGetAuthenticateField);
    CPPUNIT_TEST_SUITE_END();

public:

    void testCreator()
    {
        const char *hdr = "\r\nContent-Length: 0\r\n\r\n";
        ssize_t expectedLen = strlen(hdr);
        HttpMessage* msg = new HttpMessage();
        UtlString buf;
        ssize_t bufLen = 0;

        msg->getBytes(&buf, &bufLen);
        ASSERT_STR_EQUAL_MESSAGE("Set header comes back intact", hdr, (char *)buf.data());
        CPPUNIT_ASSERT_MESSAGE("bytes should not be null", !buf.isNull());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("buffer should contain only content length field",
                expectedLen, bufLen);
        ASSERT_STR_EQUAL_MESSAGE("buffer should contain only content length field",
                buf.data(), hdr);
        delete msg;
    }

    /**
     * Test header, message, body, message contructor
     */
    void testMessage()
    {
        // TODO break this up into several tests. Too intertwined
        const char* name = "Content-Type";
        const char* value = "text/plain";
        const char* httpTopLine = "GET /index.html HTTP/1.0";
        const char* valueRef = NULL;
        const char* n2 = "yyy";
        const char* v2 = "yyy-value";
        const char* v2a = "yyy-value2";
        UtlString messageBytes;
        UtlString messageBytes2;
        ssize_t messageLen = 0;
        ssize_t messageLen2 = 0;
        const char* body = "<HTML>\n<H3>Hello\n<BR>\n</HTML>\n";
        const HttpBody *bodyRef;
        ssize_t bodyLength = 0;
        UtlString headerLinePart;
        HttpMessage *msg;
        HttpMessage *msg2;

        msg = new HttpMessage();

        // H E A D E R
        int fieldCount = msg->getCountHeaderFields();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be zero",
                0, fieldCount);

        msg->addHeaderField(name, value);
        fieldCount = msg->getCountHeaderFields();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be zero", 1,
                fieldCount);

        valueRef = msg->getHeaderValue(0);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value,
                valueRef);

        msg->setFirstHeaderLine(httpTopLine);
        valueRef = msg->getFirstHeaderLine();
        ASSERT_STR_EQUAL_MESSAGE("incorrect top header line value",
                valueRef, httpTopLine);

        valueRef = msg->getHeaderValue(0, name);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value,
            valueRef);

        msg->addHeaderField(n2, v2);
        fieldCount = msg->getCountHeaderFields();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be 2", 2,
                fieldCount);

        valueRef = msg->getHeaderValue(0, n2);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", v2,
            valueRef);

        msg->addHeaderField(n2, v2a);
        fieldCount = msg->getCountHeaderFields();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be 3", 3,
                fieldCount);

        valueRef = msg->getHeaderValue(1, n2);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", v2a,
            valueRef);

        // B O D Y
        HttpBody *httpBody = new HttpBody(body, strlen(body));
        msg->setBody(httpBody);
        bodyRef = msg->getBody();
        CPPUNIT_ASSERT_MESSAGE("bad body pointer", httpBody == bodyRef);

        bodyRef->getBytes(&valueRef, &bodyLength);
        CPPUNIT_ASSERT_MESSAGE("bad body pointer", valueRef != NULL);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect body len", (ssize_t)strlen(body),
                bodyLength);
        ASSERT_STR_EQUAL_MESSAGE("incorrect body value", body, valueRef);

        const char* expectedLinePart[] = {
            "GET", "/index.html", "HTTP/1.0"
        };
        size_t n = sizeof(expectedLinePart) / sizeof(expectedLinePart[0]);

        for (size_t i = 0; i < n; i++)
        {
            msg->getFirstHeaderLinePart(i, &headerLinePart);
            CPPUNIT_ASSERT_MESSAGE("NULL header line part pointer",
                !headerLinePart.isNull());
            ASSERT_STR_EQUAL_MESSAGE("incorrect hdr line", expectedLinePart[i],
                headerLinePart.data());
            headerLinePart.remove(0);
        }

        msg->getBytes(&messageBytes, &messageLen);
        CPPUNIT_ASSERT_MESSAGE("NULL body pointer", !messageBytes.isNull());

        // message constructor
        msg2 = new HttpMessage(messageBytes.data(), messageLen);
        msg2->getBytes(&messageBytes2, &messageLen2);
        valueRef = msg2->getHeaderValue(0, name);
        ASSERT_STR_EQUAL_MESSAGE("incorrect message bytes", value, valueRef);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect message byte length",
                messageLen, messageLen2);
        delete msg2;

        delete msg;

        // AS DESIGNED: body delete is handled by delete msg
        // delete httpBody;
    }


    /**
     * Test header
     */
    void testHeader()
    {
        const char* name1 = "yyy";
        const char* value1 = "yyy-value";
        const char* valueRef = NULL;
        const char* name2 = "yyy1";
        const char* value2 = "yyy-value1";
        const char* value2a = "yyy-value2";
        const char* value2b = "yyy-value3";

	UtlBoolean  rc;

        HttpMessage *msg;

        msg = new HttpMessage();

        // H E A D E R
        int fieldCount = msg->getCountHeaderFields();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be zero",
                0, fieldCount);

        // add header field name1
        msg->addHeaderField(name1, value1);

        // get overall header field count
        fieldCount = msg->getCountHeaderFields();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be zero", 1,
                fieldCount);

        // get header field count for name1
        fieldCount = msg->getCountHeaderFields(name1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be zero", 1,
                fieldCount);

        // get header field by index
        valueRef = msg->getHeaderValue(0);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value1,
                valueRef);

        // get header field by index and name
        valueRef = msg->getHeaderValue(0, name1);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value1,
                valueRef);

        // add header field name2
        msg->addHeaderField(name2, value2);

        // get header field by name and index
        valueRef = msg->getHeaderValue(0, name2);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value2,
            valueRef);

        // add second header field name2
        msg->addHeaderField(name2, value2);

        // get header field by name and index
        valueRef = msg->getHeaderValue(0, name2);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value2,
            valueRef);

        // set second header field name2
        msg->setHeaderValue(name2, value2b, 1);

        // get header field by name and index
        valueRef = msg->getHeaderValue(0, name2);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value2,
            valueRef);

        // insert header field name2 as a second header with name2
        msg->insertHeaderField(name2, value2a, 2);

        // get header field by name and index
        valueRef = msg->getHeaderValue(0, name2);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value2,
            valueRef);

        // get overall header field count
        fieldCount = msg->getCountHeaderFields();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be 4", 4,
                fieldCount);

        // get name1 header field count
        fieldCount = msg->getCountHeaderFields(name1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be 1", 1,
                fieldCount);

        // get name2 header field count
        fieldCount = msg->getCountHeaderFields(name2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be 3", 3,
                fieldCount);

        // get header field by name and index
        valueRef = msg->getHeaderValue(0, name2);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value2,
            valueRef);

        // get header field by name and index
        valueRef = msg->getHeaderValue(1, name2);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value2a,
            valueRef);

        // get header field by index
        valueRef = msg->getHeaderValue(3);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value2b,
            valueRef);

        // remove non-existing header field
        rc = msg->removeHeader("non-exist", 36);
        CPPUNIT_ASSERT_MESSAGE("incorrect return code", rc == FALSE);

        // get overall header field count
        fieldCount = msg->getCountHeaderFields();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be 4", 4,
                fieldCount);

        // remove header field name1
        rc = msg->removeHeader(name1, 0);
        CPPUNIT_ASSERT_MESSAGE("incorrect return code", rc == TRUE);

        // get header field by index
        valueRef = msg->getHeaderValue(0);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value2,
            valueRef);

        // get name1 header field count
        fieldCount = msg->getCountHeaderFields(name1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be 0", 0,
                fieldCount);

        // remove second header field name2
        rc = msg->removeHeader(name2, 1);
        CPPUNIT_ASSERT_MESSAGE("incorrect return code", rc == TRUE);

        // get overall header field count
        fieldCount = msg->getCountHeaderFields();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be 2", 2,
                fieldCount);

        // get name2 header field count
        fieldCount = msg->getCountHeaderFields(name2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be 2", 2,
                fieldCount);

        // get header field by index
        valueRef = msg->getHeaderValue(0);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value2,
            valueRef);

        // get header field by name and index
        valueRef = msg->getHeaderValue(1, name2);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value2b,
            valueRef);

        // get non-exist header field by name and index
        valueRef = msg->getHeaderValue(2, name2);
        CPPUNIT_ASSERT_MESSAGE("non-NULL field value", valueRef == NULL);

        // remove header field name2
        rc = msg->removeHeader(name2, 1);
        CPPUNIT_ASSERT_MESSAGE("incorrect return code", rc == TRUE);

        // get overall header field count
        fieldCount = msg->getCountHeaderFields();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be 1", 1,
                fieldCount);

        // get header field by index
        valueRef = msg->getHeaderValue(0);
        CPPUNIT_ASSERT_MESSAGE("NULL field value", valueRef != NULL);
        ASSERT_STR_EQUAL_MESSAGE("incorrect field value", value2,
            valueRef);

        // remove header field name2
        rc = msg->removeHeader(name2, 0);
        CPPUNIT_ASSERT_MESSAGE("incorrect return code", rc == TRUE);

        // get overall header field count
        fieldCount = msg->getCountHeaderFields();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("field count should be 0", 0,
                fieldCount);

        delete msg;
    }


    void testSdp()
    {
        const char* sip = "INVITE 14 SIP/2.0\nContent-Type:application/sdp\n\n"
            "v=0\nm=audio 49170 RTP/AVP 0\nc=IN IP4 224.2.17.12/127";

        HttpMessage *msg = new HttpMessage(sip);
        SdpBody *sdp = (SdpBody *)msg->getBody();

        CPPUNIT_ASSERT_MESSAGE("Null sdp buffer", sdp != NULL);

        int mediaCount = sdp->getMediaSetCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect media count", 1, mediaCount);

        const char* referenceSdp =
            "v=0\r\nm=audio 49170 RTP/AVP 0\r\nc=IN IP4 224.2.17.12/127\r\n";
        const char* sdpBytes = NULL;
        ssize_t sdpByteLength = 0;
        sdp->getBytes(&sdpBytes, &sdpByteLength);
        for(ssize_t iii = 0; iii < sdpByteLength; iii++)
        {
            if(referenceSdp[iii] != sdpBytes[iii])
            {
                printf("index[%zd]: expected: %d got: %d\n",
                    iii, referenceSdp[iii], sdpBytes[iii]);
            }
        }
        CPPUNIT_ASSERT_MESSAGE("Null sdp serialized content", sdpBytes != NULL);
        CPPUNIT_ASSERT_MESSAGE("SDP does not match expected content",
            strcmp(referenceSdp, sdpBytes) == 0);

        HttpMessage* msgCopy = new HttpMessage(*msg);
        CPPUNIT_ASSERT_MESSAGE("NULL message copy", msgCopy != NULL);
        SdpBody *sdpCopy = (SdpBody *)msgCopy->getBody();
        CPPUNIT_ASSERT_MESSAGE("NULL SDP copy", sdpCopy != NULL);
        const char* sdpCopyBytes = NULL;
        ssize_t sdpCopyLen = 0;
        sdpCopy->getBytes(&sdpCopyBytes, &sdpCopyLen);
        //printf("SDP copy length: %d\n%s\n", sdpCopyLen, sdpCopyBytes);
        CPPUNIT_ASSERT_MESSAGE("Null sdp copy serialized content", sdpCopyBytes != NULL);
        CPPUNIT_ASSERT_MESSAGE("SDP does not match expected content",
            strcmp(referenceSdp, sdpCopyBytes) == 0);
    }


    void testMd5Digest()
    {
        const char* passToken = "john.salesman:sales@www/example.com:5+5=10";
        const char* nonce = "dcd98b7102dd2f0e8b11d0f600bfb0c093";
        const char* alg = HTTP_MD5_SESSION_ALGORITHM;
        const char *cnonce = NULL;
        const char * nonceCount = "00000001";
        const char* method = "GET";
        const char* qop = NULL;
        const char* uri = "/private/prices.html";
        const char *bodyDigest = NULL;
        const char* response = "739fc56a88db37aeebefe70572aeda5f";
        UtlString responseToken;

        HttpMessage::buildMd5Digest(passToken, alg, nonce, cnonce,  nonceCount,
                                   qop, method, uri, bodyDigest, &responseToken);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("httpmessage digest test",
            0, responseToken.compareTo(response));
    }

  void testEscape()
  {
    const char* umlautUriValue = "\"Thomas Frölich Snom 320\" <sip:2012@pforzheim.org;transport=udp>;tag=zmbmylm36n";

    UtlString umlautString(umlautUriValue);

    ASSERT_STR_EQUAL(umlautUriValue, umlautString.data());

    HttpMessage::escape(umlautString);
    const char* umlautUriEscaped = "%22Thomas+Fr%C3%B6lich+Snom+320%22+%3Csip%3A2012%40pforzheim.org%3Btransport%3Dudp%3E%3Btag%3Dzmbmylm36n";
    ASSERT_STR_EQUAL(umlautUriEscaped, umlautString.data());

    HttpMessage::unescape(umlautString);
    ASSERT_STR_EQUAL(umlautUriValue, umlautString.data());
  }

   void testGetAuthenticateField()
      {
         UtlString value;

         HttpMessage proxyRspOne(
            "SIP/2.0 407 Proxy Authentication Required\r\n"
            "From: <sip:9496722379@example.com>;tag=1c1198308561\r\n"
            "To: <sip:9499291387@example.com;user=phone>\r\n"
            "Call-Id: 119830821391200002619@10.139.33.244\r\n"
            "Cseq: 1 INVITE\r\n"
            "Via: SIP/2.0/UDP 10.139.4.84;branch=z9hG4bK-de2c934952294f774ee0acbc133e9b1d\r\n"
            "Via: SIP/2.0/UDP 10.139.33.244;branch=z9hG4bKac1198312375\r\n"
            "Proxy-Authenticate: Digest realm=\"example.com\", nonce=\"c6c882469d27a9e9c4f75ab3fca4f7921184787266\"\r\n"
            "Server: sipX/3.6.6 sipX/authproxy (Linux)\r\n"
            "Date: Wed, 18 Jul 2007 19:34:26 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n"
                                 );

         CPPUNIT_ASSERT(!proxyRspOne.getAuthenticateField(0,HttpMessage::SERVER,value));
         CPPUNIT_ASSERT(value.isNull());
         CPPUNIT_ASSERT(!proxyRspOne.getAuthenticateField(1,HttpMessage::SERVER,value));
         CPPUNIT_ASSERT(value.isNull());

         CPPUNIT_ASSERT(proxyRspOne.getAuthenticateField(0,HttpMessage::PROXY,value));
         ASSERT_STR_EQUAL("Digest realm=\"example.com\", nonce=\"c6c882469d27a9e9c4f75ab3fca4f7921184787266\"",
                          value.data());

         CPPUNIT_ASSERT(!proxyRspOne.getAuthenticateField(1,HttpMessage::PROXY,value));
         CPPUNIT_ASSERT(value.isNull());

         HttpMessage proxyRspTwo(
            "SIP/2.0 407 Proxy Authentication Required\r\n"
            "From: <sip:9496722379@example.com>;tag=1c1198308561\r\n"
            "To: <sip:9499291387@example.com;user=phone>\r\n"
            "Call-Id: 119830821391200002619@10.139.33.244\r\n"
            "Cseq: 1 INVITE\r\n"
            "Via: SIP/2.0/UDP 10.139.4.84;branch=z9hG4bK-de2c934952294f774ee0acbc133e9b1d\r\n"
            "Via: SIP/2.0/UDP 10.139.33.244;branch=z9hG4bKac1198312375\r\n"
            "Proxy-Authenticate: Digest realm=\"example.com\", nonce=\"c6c882469d27a9e9c4f75ab3fca4f7921184787266\"\r\n"
            "Proxy-Authenticate: Digest realm=\"example.net\", nonce=\"nothernonce\"\r\n"
            "Server: sipX/3.6.6 sipX/authproxy (Linux)\r\n"
            "Date: Wed, 18 Jul 2007 19:34:26 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n"
                                 );

         CPPUNIT_ASSERT(!proxyRspTwo.getAuthenticateField(0,HttpMessage::SERVER,value));
         CPPUNIT_ASSERT(value.isNull());
         CPPUNIT_ASSERT(!proxyRspTwo.getAuthenticateField(1,HttpMessage::SERVER,value));
         CPPUNIT_ASSERT(value.isNull());
         CPPUNIT_ASSERT(!proxyRspTwo.getAuthenticateField(2,HttpMessage::SERVER,value));
         CPPUNIT_ASSERT(value.isNull());

         CPPUNIT_ASSERT(proxyRspTwo.getAuthenticateField(0,HttpMessage::PROXY,value));
         ASSERT_STR_EQUAL("Digest realm=\"example.com\", nonce=\"c6c882469d27a9e9c4f75ab3fca4f7921184787266\"",
                          value.data());
         CPPUNIT_ASSERT(proxyRspTwo.getAuthenticateField(1,HttpMessage::PROXY,value));
         ASSERT_STR_EQUAL("Digest realm=\"example.net\", nonce=\"nothernonce\"",
                          value.data());

         CPPUNIT_ASSERT(!proxyRspTwo.getAuthenticateField(2,HttpMessage::PROXY,value));
         CPPUNIT_ASSERT(value.isNull());

         HttpMessage serverRspOne(
            "SIP/2.0 401 Authentication Required\r\n"
            "From: <sip:9496722379@example.com>;tag=1c1198308561\r\n"
            "To: <sip:9499291387@example.com;user=phone>\r\n"
            "Call-Id: 119830821391200002619@10.139.33.244\r\n"
            "Cseq: 1 INVITE\r\n"
            "Via: SIP/2.0/UDP 10.139.4.84;branch=z9hG4bK-de2c934952294f774ee0acbc133e9b1d\r\n"
            "Via: SIP/2.0/UDP 10.139.33.244;branch=z9hG4bKac1198312375\r\n"
            "WWW-Authenticate: Digest realm=\"example.com\", nonce=\"c6c882469d27a9e9c4f75ab3fca4f7921184787266\"\r\n"
            "Server: sipX/3.6.6 sipX/authproxy (Linux)\r\n"
            "Date: Wed, 18 Jul 2007 19:34:26 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n"
                                 );

         CPPUNIT_ASSERT(!serverRspOne.getAuthenticateField(0,HttpMessage::PROXY,value));
         CPPUNIT_ASSERT(value.isNull());
         CPPUNIT_ASSERT(!serverRspOne.getAuthenticateField(1,HttpMessage::PROXY,value));
         CPPUNIT_ASSERT(value.isNull());

         CPPUNIT_ASSERT(serverRspOne.getAuthenticateField(0,HttpMessage::SERVER,value));
         ASSERT_STR_EQUAL("Digest realm=\"example.com\", nonce=\"c6c882469d27a9e9c4f75ab3fca4f7921184787266\"",
                          value.data());

         CPPUNIT_ASSERT(!serverRspOne.getAuthenticateField(1,HttpMessage::SERVER,value));
         CPPUNIT_ASSERT(value.isNull());

         HttpMessage serverRspTwo(
            "SIP/2.0 401 Authentication Required\r\n"
            "From: <sip:9496722379@example.com>;tag=1c1198308561\r\n"
            "To: <sip:9499291387@example.com;user=phone>\r\n"
            "Call-Id: 119830821391200002619@10.139.33.244\r\n"
            "Cseq: 1 INVITE\r\n"
            "Via: SIP/2.0/UDP 10.139.4.84;branch=z9hG4bK-de2c934952294f774ee0acbc133e9b1d\r\n"
            "Via: SIP/2.0/UDP 10.139.33.244;branch=z9hG4bKac1198312375\r\n"
            "WWW-Authenticate: Digest realm=\"example.com\", nonce=\"c6c882469d27a9e9c4f75ab3fca4f7921184787266\"\r\n"
            "WWW-Authenticate: Digest realm=\"example.net\", nonce=\"nothernonce\"\r\n"
            "Server: sipX/3.6.6 sipX/authproxy (Linux)\r\n"
            "Date: Wed, 18 Jul 2007 19:34:26 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n"
                                 );

         CPPUNIT_ASSERT(!serverRspTwo.getAuthenticateField(0,HttpMessage::PROXY,value));
         CPPUNIT_ASSERT(value.isNull());
         CPPUNIT_ASSERT(!serverRspTwo.getAuthenticateField(1,HttpMessage::PROXY,value));
         CPPUNIT_ASSERT(value.isNull());
         CPPUNIT_ASSERT(!serverRspTwo.getAuthenticateField(2,HttpMessage::SERVER,value));
         CPPUNIT_ASSERT(value.isNull());

         CPPUNIT_ASSERT(serverRspTwo.getAuthenticateField(0,HttpMessage::SERVER,value));
         ASSERT_STR_EQUAL("Digest realm=\"example.com\", nonce=\"c6c882469d27a9e9c4f75ab3fca4f7921184787266\"",
                          value.data());
         CPPUNIT_ASSERT(serverRspTwo.getAuthenticateField(1,HttpMessage::SERVER,value));
         ASSERT_STR_EQUAL("Digest realm=\"example.net\", nonce=\"nothernonce\"",
                          value.data());

         CPPUNIT_ASSERT(!serverRspTwo.getAuthenticateField(2,HttpMessage::SERVER,value));
         CPPUNIT_ASSERT(value.isNull());

         HttpMessage proxyRspMixed(
            "SIP/2.0 407 Proxy Authentication Required\r\n"
            "From: <sip:9496722379@example.com>;tag=1c1198308561\r\n"
            "To: <sip:9499291387@example.com;user=phone>\r\n"
            "Call-Id: 119830821391200002619@10.139.33.244\r\n"
            "Cseq: 1 INVITE\r\n"
            "Via: SIP/2.0/UDP 10.139.4.84;branch=z9hG4bK-de2c934952294f774ee0acbc133e9b1d\r\n"
            "Via: SIP/2.0/UDP 10.139.33.244;branch=z9hG4bKac1198312375\r\n"
            "Proxy-Authenticate: Digest realm=\"example.com\", nonce=\"c6c882469d27a9e9c4f75ab3fca4f7921184787266\"\r\n"
            "WWW-Authenticate: Digest realm=\"example.net\", nonce=\"nothernonce\"\r\n"
            "Server: sipX/3.6.6 sipX/authproxy (Linux)\r\n"
            "Date: Wed, 18 Jul 2007 19:34:26 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n"
                                 );

         CPPUNIT_ASSERT(proxyRspMixed.getAuthenticateField(0,HttpMessage::SERVER,value));
         ASSERT_STR_EQUAL("Digest realm=\"example.net\", nonce=\"nothernonce\"",
                          value.data());

         CPPUNIT_ASSERT(!proxyRspMixed.getAuthenticateField(1,HttpMessage::SERVER,value));
         CPPUNIT_ASSERT(value.isNull());

         CPPUNIT_ASSERT(proxyRspMixed.getAuthenticateField(0,HttpMessage::PROXY,value));
         ASSERT_STR_EQUAL("Digest realm=\"example.com\", nonce=\"c6c882469d27a9e9c4f75ab3fca4f7921184787266\"",
                          value.data());

         CPPUNIT_ASSERT(!proxyRspMixed.getAuthenticateField(1,HttpMessage::PROXY,value));
         CPPUNIT_ASSERT(value.isNull());

      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(HttpMessageTest);
