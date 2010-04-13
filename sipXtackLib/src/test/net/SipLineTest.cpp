//
//
// Copyright (C) 2007, 2010 Avaya, Inc., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
#include <utl/UtlHashMap.h>

#include <os/OsDefs.h>
#include <net/SipLineMgr.h>
#include <net/SipLine.h>
#include <net/SipMessage.h>


/**
 * Unittest for SipLine / SipLineMgr
 */
class SipLineTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SipLineTest);
      CPPUNIT_TEST(sanityTest);
      CPPUNIT_TEST(matchUserId);
      CPPUNIT_TEST(matchIdentity);
      CPPUNIT_TEST(matchLineId);
      CPPUNIT_TEST(testLineManagerAddLine);
      CPPUNIT_TEST(testLineManagerAuthLineSelect);
      CPPUNIT_TEST_SUITE_END();

public:

    // Basic santity checking for lines; make sure they work as expected
    void sanityTest()
    {
        Url identity("\"Display Name\" <sip:userId@example.com>", Url::NameAddr, NULL) ;
        SipLine line(identity, identity, "userId") ;

        Url url = line.getIdentity() ;
        ASSERT_STR_EQUAL(identity.toString().data(), url.toString().data()) ;
        line.getPreferredContactUri(url) ;
        CPPUNIT_ASSERT(url.isUserHostPortEqual(url, SIP_PORT) == true) ;
        UtlString lineId = line.getLineId() ;
        UtlString lineParamId;
        url.getUrlParameter(SIP_LINE_IDENTIFIER, lineParamId) ;
        ASSERT_STR_EQUAL(lineParamId.data(), lineId.data()) ;
    }

    // Test the SipLine::matchesUserId function
    void matchUserId()
    {
        Url identity("\"Display Name\" <sip:userId@example.com>", Url::NameAddr, NULL) ;

        // Normal line definitions
        SipLine line(identity, identity, "userId") ;
        CPPUNIT_ASSERT(line.matchesUserId("") == false) ;
        CPPUNIT_ASSERT(line.matchesUserId("USERID") == false) ;
        CPPUNIT_ASSERT(line.matchesUserId("userId") == true) ;
        CPPUNIT_ASSERT(line.matchesUserId("xyz123") == false) ;

        // Addition of an alias
        Url alias("\"Display Name\" <sip:alias@example.com>", Url::NameAddr, NULL) ;
        line.addAlias(alias) ;
        CPPUNIT_ASSERT(line.matchesUserId("") == false) ;
        CPPUNIT_ASSERT(line.matchesUserId("USERID") == false) ;
        CPPUNIT_ASSERT(line.matchesUserId("userId") == true) ;
        CPPUNIT_ASSERT(line.matchesUserId("alias") == true) ;
        CPPUNIT_ASSERT(line.matchesUserId("xyz123") == false) ;

        // Alias with no userId
        Url alias2("\"Display Name\" <sip:127.0.0.1>", Url::NameAddr, NULL) ;
        line.addAlias(alias2) ;
        CPPUNIT_ASSERT(line.matchesUserId("") == true) ;
        CPPUNIT_ASSERT(line.matchesUserId("USERID") == false) ;
        CPPUNIT_ASSERT(line.matchesUserId("userId") == true) ;
        CPPUNIT_ASSERT(line.matchesUserId("alias") == true) ;
        CPPUNIT_ASSERT(line.matchesUserId("xyz123") == false) ;
    }

    // Test the SipLine::matchesIdentity function
    void matchIdentity()
    {
        Url preferredContact;
        Url identity("\"Display Name\" <sip:userId@example.com>", Url::NameAddr, NULL) ;
        SipLine line(identity, identity, "userId") ;
        line.getPreferredContactUri(preferredContact) ;

        CPPUNIT_ASSERT(line.matchesIdentity("") == false) ;
        CPPUNIT_ASSERT(line.matchesIdentity(identity) == true) ;
        CPPUNIT_ASSERT(line.matchesIdentity(Url("sip:userId@example.com", Url::NameAddr, NULL)) == true) ;
        CPPUNIT_ASSERT(line.matchesIdentity(Url("<sip:userId@example.com:5060>", Url::NameAddr, NULL)) == true) ;
        CPPUNIT_ASSERT(line.matchesIdentity(Url("<sip:userId@example.com:9999>", Url::NameAddr, NULL)) == false) ;
        CPPUNIT_ASSERT(line.matchesIdentity(Url("sip:xyz123@example.com", Url::NameAddr, NULL)) == false) ;
        CPPUNIT_ASSERT(line.matchesIdentity(Url("sip:example.com", Url::NameAddr, NULL)) == false) ;
        CPPUNIT_ASSERT(line.matchesIdentity(Url("sip:userId@example_com", Url::NameAddr, NULL)) == false) ;
        CPPUNIT_ASSERT(line.matchesIdentity(preferredContact) == true) ;

        // Addition of an alias
        Url alias("\"Display Name\" <sip:alias@example.com>", Url::NameAddr, NULL) ;
        line.addAlias(alias) ;
        CPPUNIT_ASSERT(line.matchesIdentity(identity) == true) ;
        CPPUNIT_ASSERT(line.matchesIdentity(Url("sip:userId@example.com", Url::NameAddr, NULL)) == true) ;
        CPPUNIT_ASSERT(line.matchesIdentity(Url("<sip:userId@example.com:5060>", Url::NameAddr, NULL)) == true) ;
        CPPUNIT_ASSERT(line.matchesIdentity(Url("<sip:userId@example.com:9999>", Url::NameAddr, NULL)) == false) ;
        CPPUNIT_ASSERT(line.matchesIdentity(Url("sip:xyz123@example.com", Url::NameAddr, NULL)) == false) ;
        CPPUNIT_ASSERT(line.matchesIdentity(Url("sip:example.com", Url::NameAddr, NULL)) == false) ;
        CPPUNIT_ASSERT(line.matchesIdentity(Url("sip:userId@example_com", Url::NameAddr, NULL)) == false) ;
        CPPUNIT_ASSERT(line.matchesIdentity(preferredContact) == true) ;
        CPPUNIT_ASSERT(line.matchesIdentity(alias) == true) ;
        CPPUNIT_ASSERT(line.matchesIdentity(Url("sip:example.com", Url::NameAddr, NULL)) == false) ;

        // Alias with no userId
        Url alias2("\"Display Name\" <sip:127.0.0.1>", Url::NameAddr, NULL) ;
        line.addAlias(alias2) ;
        CPPUNIT_ASSERT(line.matchesIdentity(identity) == true) ;
        CPPUNIT_ASSERT(line.matchesIdentity(alias) == true) ;
        CPPUNIT_ASSERT(line.matchesIdentity(alias2) == true) ;
        CPPUNIT_ASSERT(line.matchesIdentity(Url("sip:userId@example.com", Url::NameAddr, NULL)) == true) ;
        CPPUNIT_ASSERT(line.matchesIdentity(Url("sip:example.com", Url::NameAddr, NULL)) == false) ;
    }

    // Test the SipLine::matchesLineId function
    void matchLineId()
    {
        Url identity("\"Display Name\" <sip:userId@example.com>", Url::NameAddr, NULL) ;
        SipLine line(identity, identity, "userId") ;

        CPPUNIT_ASSERT(line.matchesLineId("") == false) ;
        CPPUNIT_ASSERT(line.matchesLineId(line.getLineId()) == true) ;
        CPPUNIT_ASSERT(line.matchesLineId("xyz123") == false) ;
    }

    // Test line adding functions from the SipLineMgr
    void testLineManagerAddLine()
    {
        UtlString lineId ;
        Url identity("\"Display Name\" <sip:userId@example.com>", Url::NameAddr, NULL) ;
        Url alias("\"Display Name\" <sip:alias@example.com>", Url::NameAddr, NULL) ;
        char cTemp[256] ;
        UtlBoolean bRC ;
        int iRC ;

        SipLineMgr mgr ;
        SipLine line(identity, identity, "userId") ;
        lineId = line.getLineId() ;

        // Add Line
        bRC = mgr.addLine(line, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Add alias
        bRC = mgr.addLineAlias(identity, alias) ;
        CPPUNIT_ASSERT(bRC) ;

        // Add credential
        UtlString passwordToken;
        HttpMessage::buildMd5UserPasswordDigest("userId", "testRealm", "password", passwordToken);
        bRC = mgr.addCredentialForLine(identity, "testRealm", "userId", passwordToken, HTTP_DIGEST_AUTHENTICATION) ;
        CPPUNIT_ASSERT(bRC) ;


        // Sanity Checking
        iRC = mgr.getNumLines() ;
        CPPUNIT_ASSERT(iRC == 1) ;
        iRC = mgr.getNumOfCredentialsForLine(identity) ;
        CPPUNIT_ASSERT(iRC == 1) ;
        iRC = mgr.getNumOfCredentialsForLine(alias) ;
        CPPUNIT_ASSERT(iRC == 1) ;

        SipLine test ;

        //
        // Test getLine via "To" URL
        //
        bRC = mgr.getLine("sip:userId@example.com", "", "", test) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = mgr.getLine("sip:alias@example.com", "", "", test) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = mgr.getLine("sip:bogus@example.com", "", "", test) ;
        CPPUNIT_ASSERT(!bRC) ;
        bRC = mgr.getLine("sip:example.com", "", "", test) ;
        CPPUNIT_ASSERT(!bRC) ;
        bRC = mgr.getLine("sip:userId@127.0.0.1", "", "", test) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = mgr.getLine("sip:alias@127.0.0.1", "", "", test) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = mgr.getLine("sip:bogus@127.0.0.1", "", "", test) ;
        CPPUNIT_ASSERT(!bRC) ;
        bRC = mgr.getLine("sip:127.0.0.1", "", "", test) ;
        CPPUNIT_ASSERT(!bRC) ;
        sprintf(cTemp, "<sip:127.0.0.1;LINEID=%s>", lineId.data()) ;
        bRC = mgr.getLine(cTemp, "", "", test) ;
        CPPUNIT_ASSERT(bRC) ;

        //
        // Test getLine via local contact
        //
        bRC = mgr.getLine("", "sip:userId@example.com", "", test) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = mgr.getLine("", "sip:alias@example.com", "", test) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = mgr.getLine("", "sip:bogus@example.com", "", test) ;
        CPPUNIT_ASSERT(!bRC) ;
        bRC = mgr.getLine("", "sip:example.com", "", test) ;
        CPPUNIT_ASSERT(!bRC) ;
        bRC = mgr.getLine("", "sip:userId@127.0.0.1", "", test) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = mgr.getLine("", "sip:alias@127.0.0.1", "", test) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = mgr.getLine("", "sip:bogus@127.0.0.1", "", test) ;
        CPPUNIT_ASSERT(!bRC) ;
        bRC = mgr.getLine("", "sip:127.0.0.1", "", test) ;
        CPPUNIT_ASSERT(!bRC) ;
        sprintf(cTemp, "<sip:127.0.0.1;LINEID=%s>", lineId.data()) ;
        bRC = mgr.getLine("", cTemp, "", test) ;
        CPPUNIT_ASSERT(bRC) ;

        //
        // Test getLine via request URI
        //
        bRC = mgr.getLine("", "", "sip:userId@example.com", test) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = mgr.getLine("", "", "sip:alias@example.com", test) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = mgr.getLine("", "", "sip:bogus@example.com", test) ;
        CPPUNIT_ASSERT(!bRC) ;
        bRC = mgr.getLine("", "", "sip:example.com", test) ;
        CPPUNIT_ASSERT(!bRC) ;
        bRC = mgr.getLine("", "", "sip:userId@127.0.0.1", test) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = mgr.getLine("", "", "sip:alias@127.0.0.1", test) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = mgr.getLine("", "", "sip:bogus@127.0.0.1", test) ;
        CPPUNIT_ASSERT(!bRC) ;
        bRC = mgr.getLine("", "", "sip:127.0.0.1", test) ;
        CPPUNIT_ASSERT(!bRC) ;
        sprintf(cTemp, "sip:127.0.0.1;LINEID=%s", lineId.data()) ;
        bRC = mgr.getLine("", "", cTemp, test) ;
        CPPUNIT_ASSERT(bRC) ;
    }

    void buildRequestAndResponse(const Url& from,
                                 const Url& contact,
                                 SipMessage& request,
                                 SipMessage& response)
    {
        char cTemp[2048] ;

        const char* requestTemplate = \
            "REFER sip:foo@example.com\r\n"
            "Route: <sip:127.0.0.1:5060;lr;sipXecs-rs=%%2Afrom%%7ERkI5Mjc4NjktNjY4QjM4NzQ%%60.400_authrules%%2Aauth%%7E%%212e6c4f0639ffe4ffbf878dc06fd5af09>\r\n"
            "To: \"To Name\" <sip:foo@example.com>;tag=1234\r\n"
            "From: %s\r\n"
            "Call-Id: 20c4578d-e9a84853-1f3b28b6@127.0.0.1\r\n"
            "Cseq: 2 REFER\r\n"
            "Contact: %s\r\n"
            "Referred-By: <sip:666@example.com;user=phone>\r\n"
            "Refer-To: <sip:100@example.com>\r\n"
            "Date: Fri, 28 Dec 2007 21:19:24 GMT\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipXacd (3.9.7-011568) (Linux)\r\n"
            "Accept-Language: en\r\n"
            "Allow: INVITE, ACK, CANCEL, BYE, REFER, OPTIONS, INFO, REGISTER, SUBSCRIBE, NOTIFY\r\n"
            "Supported: replaces\r\n"
            "Via: SIP/2.0/UDP 127.0.0.1:5150;branch=z9hG4bK-sipX-000a66399056f9864f015727ce351eb7e7ec\r\n"
            "Content-Length: 0\n"
            "\r\n" ;

        sprintf(cTemp, requestTemplate,
                from.toString().data(),
                contact.toString().data()) ;

        request.parseMessage(cTemp, strlen(cTemp)) ;

        const char* responseTemplate = \
            "SIP/2.0 407 Proxy Authentication Required\r\n"
            "Server: sipX/3.9.7 sipX/sipXproxy (Linux)\r\n"
            "To: \"To Name\" <sip:foo@example.com>;tag=1234\r\n"
            "From: %s\r\n"
            "Call-Id: 20c4578d-e9a84853-1f3b28b6@127.0.0.1\r\n"
            "Cseq: 2 REFER\r\n"
            "Via: SIP/2.0/UDP 127.0.0.1:5150;branch=z9hG4bK-sipX-000a66399056f9864f015727ce351eb7e7ec\r\n"
            "Proxy-Authenticate: Digest realm=\"testRealm\", nonce=\"7af796108789cb42ee15c19a98b273ed4775685c\"\r\n"
            "Date: Fri, 28 Dec 2007 21:19:24 GMT\r\n"
            "Contact: <sip:127.0.0.1:5060>\r\n"
            "Content-Length: 0\r\n"
            "\r\n" ;

        sprintf(cTemp, responseTemplate,
                from.toString().data()) ;

        response.parseMessage(cTemp, strlen(cTemp)) ;
    }

    // Test matching line authentication credentials
    void testLineManagerAuthLineSelect()
    {
        UtlString lineId ;
        Url identity("\"Display Name\" <sip:userId@example.com>", Url::NameAddr, NULL) ;
        Url alias("\"Display Name\" <sip:alias@example.com>", Url::NameAddr, NULL) ;
        UtlBoolean bRC ;
        Url from ;
        Url contact ;
        SipLineMgr mgr ;

        // Add line
        SipLine line(identity, identity, "userId") ;
        lineId = line.getLineId() ;
        bRC = mgr.addLine(line, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Add alias
        bRC = mgr.addLineAlias(identity, alias) ;
        CPPUNIT_ASSERT(bRC) ;

        // Add credential
        UtlString passwordToken;
        HttpMessage::buildMd5UserPasswordDigest("userId", "testRealm", "password", passwordToken);
        bRC = mgr.addCredentialForLine(identity, "testRealm", "userId", passwordToken, HTTP_DIGEST_AUTHENTICATION) ;
        CPPUNIT_ASSERT(bRC) ;

        // Expected From and Contact
        {
            SipMessage request ;
            SipMessage response ;
            SipMessage requestWithAuth;

            from = identity ;
            from.setFieldParameter("tag", "5678") ;
            line.getPreferredContactUri(contact) ;
            buildRequestAndResponse(from, contact, request, response) ;
            bRC = mgr.buildAuthorizationRequest(&response, &request, &requestWithAuth);
            CPPUNIT_ASSERT(bRC) ;
        }

        // Expected From and Contact w/o lineId
        {
            SipMessage request ;
            SipMessage response ;
            SipMessage requestWithAuth;

            from = identity ;
            from.setFieldParameter("tag", "5678") ;
            contact = identity ;
            buildRequestAndResponse(from, contact, request, response) ;
            bRC = mgr.buildAuthorizationRequest(&response, &request, &requestWithAuth);
            CPPUNIT_ASSERT(bRC) ;
        }

        // Unknown identity and Unknown Contact
        {
            SipMessage request ;
            SipMessage response ;
            SipMessage requestWithAuth;

            from = Url("<sip:unknown@example.com>;tag=5678", Url::NameAddr, NULL) ;
            contact = Url("sip:unknown@example.com", Url::NameAddr, NULL) ;
            buildRequestAndResponse(from, contact, request, response) ;
            bRC = mgr.buildAuthorizationRequest(&response, &request, &requestWithAuth);
            CPPUNIT_ASSERT(!bRC) ;
        }

        // Unknown identity and Unknown Contact w/ LineId
        {
            SipMessage request ;
            SipMessage response ;
            SipMessage requestWithAuth;

            from = Url("<sip:unknown@example.com>;tag=5678", Url::NameAddr, NULL) ;
            contact = Url("sip:unknown@example.com", Url::NameAddr, NULL) ;
            contact.setUrlParameter("LINEID", lineId) ;
            buildRequestAndResponse(from, contact, request, response) ;
            bRC = mgr.buildAuthorizationRequest(&response, &request, &requestWithAuth);
            CPPUNIT_ASSERT(bRC) ;
        }

        // Unknown identity and IP w/ LineId
        {
            SipMessage request ;
            SipMessage response ;
            SipMessage requestWithAuth;

            from = Url("<sip:unknown@example.com>;tag=5678", Url::NameAddr, NULL) ;
            contact = Url("sip:127.0.0.1", Url::NameAddr, NULL) ;
            contact.setUrlParameter("LINEID", lineId) ;
            buildRequestAndResponse(from, contact, request, response) ;
            bRC = mgr.buildAuthorizationRequest(&response, &request, &requestWithAuth);
            CPPUNIT_ASSERT(bRC) ;
        }

        // Expected identity and Unknown contact
        {
            SipMessage request ;
            SipMessage response ;
            SipMessage requestWithAuth;

            from = identity ;
            from.setFieldParameter("tag", "5678") ;
            contact = Url("sip:127.0.0.1", Url::NameAddr, NULL) ;
            buildRequestAndResponse(from, contact, request, response) ;
            bRC = mgr.buildAuthorizationRequest(&response, &request, &requestWithAuth);
            CPPUNIT_ASSERT(bRC) ;
        }

        // Alias identity and Unknown contact
        {
            SipMessage request ;
            SipMessage response ;
            SipMessage requestWithAuth;

            from = alias ;
            from.setFieldParameter("tag", "5678") ;
            contact = Url("sip:127.0.0.1", Url::NameAddr, NULL) ;
            buildRequestAndResponse(from, contact, request, response) ;
            bRC = mgr.buildAuthorizationRequest(&response, &request, &requestWithAuth);
            CPPUNIT_ASSERT(bRC) ;
        }

        // Unknown identity and alias contact
        {
            SipMessage request ;
            SipMessage response ;
            SipMessage requestWithAuth ;

            from = Url("<sip:unknown@example.com>;tag=5678", Url::NameAddr, NULL) ;
            contact = alias;
            buildRequestAndResponse(from, contact, request, response) ;
            bRC = mgr.buildAuthorizationRequest(&response, &request, &requestWithAuth);
            CPPUNIT_ASSERT(bRC) ;
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipLineTest);
