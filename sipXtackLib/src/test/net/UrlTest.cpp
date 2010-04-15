//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <string.h>
#include <net/Url.h>
#include <net/HttpMessage.h>
#include <net/NetMd5Codec.h>
#include <net/SipMessage.h>
#include <utl/UtlTokenizer.h>

#include "os/OsTimeLog.h"

#define MISSING_PARAM  "---missing---"

#define ASSERT_ARRAY_MESSAGE(message, expected, actual) \
  UrlTest::assertArrayMessage((expected),(actual), \
      CPPUNIT_SOURCELINE(), (message))

class UrlTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(UrlTest);
    CPPUNIT_TEST(testSchemes);
    CPPUNIT_TEST(testFileBasic);
    CPPUNIT_TEST(testFileWithPortAndPath);
    CPPUNIT_TEST(testHttpBasic);
    CPPUNIT_TEST(testHttpWithPortAndPath);
    CPPUNIT_TEST(testHttpWithQuery);
    CPPUNIT_TEST(testHttpWithQueryNameAddr);
    CPPUNIT_TEST(testHttpWithQueryAddrSpec);
    CPPUNIT_TEST(testSipBasic);
    CPPUNIT_TEST(testSipBasicWithPort);
    CPPUNIT_TEST(testIpBasicWithBrackets);
    CPPUNIT_TEST(testSemiHeaderParam);
    CPPUNIT_TEST(testSipParametersWithComma);
    CPPUNIT_TEST(testSipParametersWithCommaPlusMore);
    CPPUNIT_TEST(testSipAdvanced);
    CPPUNIT_TEST(testSipComplexUser);
    CPPUNIT_TEST(testLongHostname);
    CPPUNIT_TEST(testSipParameters);
    CPPUNIT_TEST(testFieldParameterWhitespace);
    CPPUNIT_TEST(testFullSip);
    CPPUNIT_TEST(testQuotedName);
    CPPUNIT_TEST(testFancyNames);
    CPPUNIT_TEST(testEncoded);
    CPPUNIT_TEST(testNoFieldParams);
    CPPUNIT_TEST(testNoHeaderParams);
    CPPUNIT_TEST(testCorrection);
    CPPUNIT_TEST(testIpAddressOnly);
    CPPUNIT_TEST(testMissingAngles);
    CPPUNIT_TEST(testNoAngleParam);
    CPPUNIT_TEST(testHostAddressOnly);
    CPPUNIT_TEST(testHostAndPort);
    CPPUNIT_TEST(testIPv6Host);
    CPPUNIT_TEST(testBogusPort);
    CPPUNIT_TEST(testMultiple);
    CPPUNIT_TEST(testNoBracketUrlWithAllParamsWithVaryingSpace);
    CPPUNIT_TEST(testConstruction);
    CPPUNIT_TEST(testHttpConstruction);
    CPPUNIT_TEST(testComplexConstruction);
    CPPUNIT_TEST(testAddAttributesToExisting);
    CPPUNIT_TEST(testComplexDisplayName);
    CPPUNIT_TEST(testChangeValues);
    CPPUNIT_TEST(testRemoveAttributes);
    CPPUNIT_TEST(testRemoveUrlParameterCase);
    CPPUNIT_TEST(testRemoveFieldParameterCase);
    CPPUNIT_TEST(testRemoveHeaderParameterCase);
    CPPUNIT_TEST(testRemoveAllTypesOfAttributes);
    CPPUNIT_TEST(testRemoveAngleBrackets);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST(testAssignment);
    CPPUNIT_TEST(testGetAllParameters);
    CPPUNIT_TEST(testGetDuplicateNamedParameters);
    CPPUNIT_TEST(testGetOnlyUrlParameters);
    CPPUNIT_TEST(testGetOnlyHeaderParameters);
    CPPUNIT_TEST(testGetOnlyFieldParameters);
    CPPUNIT_TEST(testGetUrlParameterCase);
    CPPUNIT_TEST(testGetFieldParameterCase);
    CPPUNIT_TEST(testGetHeaderParameterCase);
    CPPUNIT_TEST(testIsDigitString);
    CPPUNIT_TEST(testIsUserHostPortEqualExact);
    CPPUNIT_TEST(testIsUserHostPortVaryCapitalization);
    CPPUNIT_TEST(testIsUserHostPortNoPort);
    CPPUNIT_TEST(testIsUserHostPortNoMatch);
    CPPUNIT_TEST(testIsUserHostPortPorts);
    CPPUNIT_TEST(testToString);
    CPPUNIT_TEST(testFromString);
    CPPUNIT_TEST(testGetIdentity);
    CPPUNIT_TEST(testNormalUri);
    CPPUNIT_TEST(testBigUriDisplayName);
    CPPUNIT_TEST(testBigUriQuotedName);
    CPPUNIT_TEST(testBigUriScheme);
    CPPUNIT_TEST(testBigUriUser);
    CPPUNIT_TEST(testBigUriNoSchemeUser);
    CPPUNIT_TEST(testBigUriHost);
    CPPUNIT_TEST(testGRUU);
    CPPUNIT_TEST(testErrors);
    CPPUNIT_TEST_SUITE_END();

private:

    UtlString *assertValue;

    char msg[1024];

   static const size_t BIG_SIZE=8200;
   UtlString bigtoken; // bigtoken = BIG_SIZE characters to be used in constructing strings

public:

   void setUp()
    {
        assertValue = new UtlString();

        CPPUNIT_ASSERT(bigtoken.capacity(BIG_SIZE) >= BIG_SIZE);
        while (bigtoken.length() < BIG_SIZE)
        {
           bigtoken.append("a123456789"); // use leading a to match name syntax
        }
    }

    void tearDown()
    {
        delete assertValue;
    }

    // Test all schemes that should be recognized.
    void testSchemes()
    {
       const char* unknown_s = "UNKNOWN-URL-SCHEME:xxx@yyy";
       Url unknown(unknown_s);
       CPPUNIT_ASSERT_EQUAL_MESSAGE(unknown_s,
                                    Url::UnknownUrlScheme, unknown.getScheme());

       const char* sip_s = "sip:foo@bar";
       Url sip(sip_s);
       CPPUNIT_ASSERT_EQUAL_MESSAGE(sip_s,
                                    Url::SipUrlScheme, sip.getScheme());

       const char* sips_s = "sips:xxx@yyy";
       Url sips(sips_s);
       CPPUNIT_ASSERT_EQUAL_MESSAGE(sips_s,
                                    Url::SipsUrlScheme, sips.getScheme());

       const char* http_s = "http://host/path/file";
       Url http(http_s);
       CPPUNIT_ASSERT_EQUAL_MESSAGE(http_s,
                                    Url::HttpUrlScheme, http.getScheme());

       const char* https_s = "https://host/path/file";
       Url https(https_s);
       CPPUNIT_ASSERT_EQUAL_MESSAGE(https_s,
                                    Url::HttpsUrlScheme, https.getScheme());

       const char* ftp_s = "ftp://host/path/file";
       Url ftp(ftp_s);
       CPPUNIT_ASSERT_EQUAL_MESSAGE(ftp_s,
                                    Url::FtpUrlScheme, ftp.getScheme());

       const char* file_s = "file://host/path/file";
       Url file(file_s);
       CPPUNIT_ASSERT_EQUAL_MESSAGE(file_s,
                                    Url::FileUrlScheme, file.getScheme());

       const char* mailto_s = "mailto:xxx@yyy";
       Url mailto(mailto_s);
       CPPUNIT_ASSERT_EQUAL_MESSAGE(mailto_s,
                                    Url::MailtoUrlScheme, mailto.getScheme());

       // Implicit "sip:" scheme.
       const char* implicit_s = "foo@bar";
       Url implicit(implicit_s);
       CPPUNIT_ASSERT_EQUAL_MESSAGE(implicit_s,
                                    Url::SipUrlScheme, implicit.getScheme());
    }

    void testFileBasic()
    {
        const char* szUrl =  "file://www.sipfoundry.org/dddd/ffff.txt";
#ifdef _WIN32
        KNOWN_FATAL_BUG("Returned path separator is wrong under Win32", "XSL-74");
#endif
        Url url(szUrl);
        sprintf(msg, "simple file url : %s", szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "www.sipfoundry.org", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "file", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "/dddd/ffff.txt", getPath(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, PORT_NONE, url.getHostPort());
    }

    void testFileWithPortAndPath()
    {
        const char* szUrl = "file://server:8080/dddd/ffff.txt";

#ifdef _WIN32
        KNOWN_FATAL_BUG("Returned path separator is wrong under Win32", "XSL-74");
#endif
        sprintf(msg, "file url w/path and port : %s", szUrl);
        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "server", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "file", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "/dddd/ffff.txt", getPath(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 8080, url.getHostPort());
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, getUri(url));
    }

    void testHttpBasic()
    {
        const char* szUrl =  "http://www.sipfoundry.org";

        Url url(szUrl);
        sprintf(msg, "simple http url : %s", szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "www.sipfoundry.org", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "http", getUrlType(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, PORT_NONE, url.getHostPort());
    }

   void testHttpWithPortAndPath()
      {
         const char* szUrl = "http://server:8080/dddd/ffff.txt";
#ifdef _WIN32
         KNOWN_FATAL_BUG("Returned path separator is wrong under Win32", "XSL-74");
#endif
         sprintf(msg, "url w/path and port : %s", szUrl);
         Url url(szUrl);
         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "server", getHostAddress(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "http", getUrlType(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "/dddd/ffff.txt", getPath(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "/dddd/ffff.txt", getPath(url,TRUE));
         CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 8080, url.getHostPort());
         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, getUri(url));
      }

   void testHttpWithQuery()
      {
         const char* szUrl = "http://server:8080/dddd/ffff.txt?p1=v1&p2=v2";
#ifdef _WIN32
         KNOWN_FATAL_BUG("Returned path separator is wrong under Win32", "XSL-74");
#endif
         sprintf(msg, "url w/path and port : %s", szUrl);
         Url url(szUrl);
         ASSERT_STR_EQUAL_MESSAGE(msg, "server", getHostAddress(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "http", getUrlType(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "/dddd/ffff.txt", getPath(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "/dddd/ffff.txt?p1=v1&p2=v2", getPath(url,TRUE));

         CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 8080, url.getHostPort());

         ASSERT_STR_EQUAL_MESSAGE(msg, "v1", getHeaderParam("p1",url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "v2", getHeaderParam("p2",url));

         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, getUri(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
      }

   void testHttpWithQueryNameAddr()
      {
         const char* szUrl = "https://localhost:8091/cgi-bin/voicemail/mediaserver.cgi?action=deposit&mailbox=111&from=%22Dale+Worley%22%3Csip%3A173%40pingtel.com%3E%3Btag%253D3c11304";
#ifdef _WIN32
        KNOWN_FATAL_BUG("Returned path separator is wrong under Win32", "XSL-74");
#endif
         Url url(szUrl);
         sprintf(msg, "http url with query (name-addr) : %s", szUrl);
         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));

         const char* szUrl2 = "https://localhost/mediaserver.cgi?foo=bar";

         Url url2(szUrl2);
         sprintf(msg, "http url with query (name-addr) : %s", szUrl2);
         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl2, toString(url2));
      }

   void testHttpWithQueryAddrSpec()
      {
         const char* szUrl = "https://localhost:8091/cgi-bin/voicemail/mediaserver.cgi?action=deposit&mailbox=111&from=%22Dale+Worley%22%3Csip%3A173%40pingtel.com%3E%3Btag%253D3c11304";
#ifdef _WIN32
         KNOWN_FATAL_BUG("Returned path separator is wrong under Win32", "XSL-74");
#endif
         Url url(szUrl, TRUE);
         sprintf(msg, "http url with query (addr-spec) : %s", szUrl);
         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));

         const char* szUrl2 = "https://localhost/mediaserver.cgi?foo=bar";

         Url url2(szUrl2, TRUE);
         sprintf(msg, "http url with query (addr-spec) : %s", szUrl2);
         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl2, toString(url2));
      }


   void testSipBasic()
    {
        const char* szUrl = "sip:rschaaf@10.1.1.89";

        sprintf(msg, "sip url with ip address: %s", szUrl);
        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, "10.1.1.89", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, PORT_NONE, url.getHostPort());

        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, getUri(url));
    }

    void testSipBasicWithPort()
    {
        const char* szUrl = "sip:fsmith@sipfoundry.org:5555";

        sprintf(msg, "sip url with port: %s", szUrl);
        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, "sipfoundry.org", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 5555, url.getHostPort());

        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, getUri(url));
    }

    void testIpBasicWithBrackets()
    {
        const char* szUrl = "<sip:rschaaf@sipfoundry.org>";

        sprintf(msg, "url sip address: %s", szUrl);
        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, "sipfoundry.org", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "rschaaf", getUserId(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, PORT_NONE, url.getHostPort());

        ASSERT_STR_EQUAL_MESSAGE(msg, "sip:rschaaf@sipfoundry.org", toString(url));
        url.includeAngleBrackets();

        ASSERT_STR_EQUAL_MESSAGE(msg, "<sip:rschaaf@sipfoundry.org>", toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip:rschaaf@sipfoundry.org", getUri(url));
    }

   void testSipParametersWithComma()
      {
        Url url;
        UtlString nextUri;

        // A name-addr whose field parameter contains commas, with no
        // nextUri argument.

        UtlString uri1("<sip:306@10.10.1.2>;methods=\"INVITE, ACK, BYE\"");
        sprintf(msg, "Field parameters include comma: '%s'", uri1.data());

        CPPUNIT_ASSERT_MESSAGE(msg, url.fromString(uri1, Url::NameAddr, NULL));

        ASSERT_STR_EQUAL_MESSAGE(msg, "10.10.1.2", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "306", getUserId(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, PORT_NONE, url.getHostPort());
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip:306@10.10.1.2", getUri(url));

        ASSERT_STR_EQUAL_MESSAGE(msg, uri1, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "INVITE, ACK, BYE", getFieldParam("methods", url));
      }

   void testSipParametersWithCommaPlusMore()
      {
         // A name-addr whose field parameter contains commas, with a
         // nextUri argument.
        Url url;
        UtlString nextUri;

        UtlString uri2("<sip:306@10.10.1.2>;methods=\"INVITE, ACK, BYE\", foo-bar-baz");
        sprintf(msg, "Field parameters include comma: '%s'", uri2.data());

        CPPUNIT_ASSERT_MESSAGE(msg, url.fromString(uri2, Url::NameAddr, &nextUri));

        ASSERT_STR_EQUAL_MESSAGE(msg, "10.10.1.2", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "306", getUserId(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, PORT_NONE, url.getHostPort());
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip:306@10.10.1.2", getUri(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "INVITE, ACK, BYE", getFieldParam("methods", url));

        ASSERT_STR_EQUAL_MESSAGE(msg, "<sip:306@10.10.1.2>;methods=\"INVITE, ACK, BYE\"", toString(url));

        ASSERT_STR_EQUAL_MESSAGE(msg, "foo-bar-baz", nextUri);
    }

    void testSipAdvanced()
    {
        const char* szUrl = "Rich Schaaf<sip:sip.tel.sipfoundry.org:8080>" ;

        sprintf(msg, "advanced bracketed sip address: %s", szUrl);
        Url url(szUrl);

        ASSERT_STR_EQUAL_MESSAGE(msg, "sip.tel.sipfoundry.org", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "", getUserId(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "Rich Schaaf", getDisplayName(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 8080, url.getHostPort());

        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip:sip.tel.sipfoundry.org:8080", getUri(url));
    }

    void testSipComplexUser()
    {
        const char* szUrl = "Raghu Venkataramana<sip:user-tester.my/place?"
            "&yourplace@sipfoundry.org>";

        sprintf(msg, "complex user sip address: %s", szUrl);
        Url url(szUrl);

        ASSERT_STR_EQUAL_MESSAGE(msg, "sipfoundry.org",
            getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "user-tester.my/place?&yourplace",
            getUserId(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, PORT_NONE, url.getHostPort());

        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip:user-tester.my/place?&yourplace@sipfoundry.org",
                                 getUri(url));
    }


    void testLongHostname()
    {
        const char* szUrl =
           "Raghu Venkataramana<sip:125@testing.stage.inhouse.sipfoundry.org>" ;

        sprintf(msg, "long hostname sip address: %s", szUrl);
        Url url(szUrl);

        ASSERT_STR_EQUAL_MESSAGE(msg, "testing.stage.inhouse.sipfoundry.org",
            getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "125", getUserId(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, PORT_NONE, url.getHostPort());

        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip:125@testing.stage.inhouse.sipfoundry.org", getUri(url));
    }

    void testSipParameters()
    {
        const char *szUrl = "<sip:username@10.1.1.225:555;tag=xxxxx;transport=TCP;"
            "msgId=4?call-Id=call2&cseq=2+INVITE>;fieldParam1=1234;fieldParam2=2345";

        const char *szUrlCorrected = "<sip:username@10.1.1.225:555;tag=xxxxx;"
            "transport=TCP;msgId=4?call-Id=call2&cseq=2+INVITE>;fieldParam1=1234;"
            "fieldParam2=2345";

        Url url(szUrl);
        sprintf(msg, "%s", szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrlCorrected, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "10.1.1.225", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "username", getUserId(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 555, url.getHostPort());
        ASSERT_STR_EQUAL_MESSAGE(msg, "xxxxx", getParam("tag", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "TCP", getParam("transport", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "4", getParam("msgId", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "call2", getHeaderParam("call-Id", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "2 INVITE", getHeaderParam("cseq", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "1234", getFieldParam("fieldParam1", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "2345", getFieldParam("fieldParam2", url));
    }

    void testFieldParameterWhitespace()
    {
        const char *szUrl = "<sip:username@10.1.1.225:555;tag=xxxxx;transport=TCP;"
            "msgId=4?call-Id=call2&cseq=2+INVITE> \t; \tfieldParam1 \t= \t1234 \t; \tfieldParam2 \t= \t2345";

        const char *szUrlCorrected = "<sip:username@10.1.1.225:555;tag=xxxxx;"
            "transport=TCP;msgId=4?call-Id=call2&cseq=2+INVITE>;fieldParam1=1234;"
            "fieldParam2=2345";

        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, szUrlCorrected, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "1234", getFieldParam("fieldParam1", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "2345", getFieldParam("fieldParam2", url));
    }

    void testFullSip()
    {
        const char *szUrl = "Display Name<sip:uname@sipserver:555;"
            "tag=xxxxx;transport=TCP;msgId=4?call-Id=call2&cseq=2+INVITE>;"
            "fieldParam1=1234;fieldParam2=2345";
        Url url(szUrl);
        sprintf(msg, "%s", szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, "sipserver", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "uname", getUserId(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "Display Name", getDisplayName(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 555, url.getHostPort());
        ASSERT_STR_EQUAL_MESSAGE(msg, "xxxxx", getParam("tag", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "TCP", getParam("transport", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "4", getParam("msgId", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "call2", getHeaderParam("call-Id", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "2 INVITE", getHeaderParam("cseq", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "1234", getFieldParam("fieldParam1", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "2345", getFieldParam("fieldParam2", url));

        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(
           msg,
           "sip:uname@sipserver:555;tag=xxxxx;transport=TCP;msgId=4?call-Id=call2&cseq=2+INVITE",
           getUri(url)
                                 );
    }

   void testQuotedName()
      {
         const char *szUrl = "\"Display \\\"Name\"<sip:easy@sipserver>";
         Url url(szUrl);
         sprintf(msg, "%s", szUrl);
         ASSERT_STR_EQUAL_MESSAGE(msg, "\"Display \\\"Name\"", getDisplayName(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "easy", getUserId(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "sipserver", getHostAddress(url));
         CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, PORT_NONE, url.getHostPort());

         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "sip:easy@sipserver", getUri(url));
      }

   void testFancyNames()
      {
         const char *szUrl = "\"(Display \\\"< @ Name)\"  <sip:?$,;silly/user+(name)_&=.punc%2d!bing*bang~'-@sipserver:555;"
            "tag=xxxxx;transport=TCP;msgId=4?call-Id=call2&cseq=2+INVITE>;"
            "fieldParam1=1234;fieldParam2=2345";
         const char *szUrl_corrected = "\"(Display \\\"< @ Name)\"<sip:?$,;silly/user+(name)_&=.punc%2d!bing*bang~'-@sipserver:555;"
            "tag=xxxxx;transport=TCP;msgId=4?call-Id=call2&cseq=2+INVITE>;"
            "fieldParam1=1234;fieldParam2=2345";
         Url url(szUrl);
         sprintf(msg, "%s", szUrl);
         ASSERT_STR_EQUAL_MESSAGE(msg, "\"(Display \\\"< @ Name)\"", getDisplayName(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "?$,;silly/user+(name)_&=.punc%2d!bing*bang~'-", getUserId(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "sipserver", getHostAddress(url));
         CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 555, url.getHostPort());
         ASSERT_STR_EQUAL_MESSAGE(msg, "xxxxx", getParam("tag", url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "TCP", getParam("transport", url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "4", getParam("msgId", url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "call2", getHeaderParam("call-Id", url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "2 INVITE", getHeaderParam("cseq", url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "1234", getFieldParam("fieldParam1", url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "2345", getFieldParam("fieldParam2", url));

         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl_corrected, toString(url));
         ASSERT_STR_EQUAL_MESSAGE(
            msg,
            "sip:?$,;silly/user+(name)_&=.punc%2d!bing*bang~'-@sipserver:555;"
            "tag=xxxxx;transport=TCP;msgId=4?call-Id=call2&cseq=2+INVITE",
            getUri(url));
      }

   void testEncoded()
    {
        const char *szUrl = "D Name<sip:autoattendant@sipfoundry.org:5100;"
            "play=http%3A%2F%2Flocalhost%3A8090%2Fsipx-cgi%2Fmediaserver.cgi"
            "%3Faction%3Dautoattendant?hp1=hval1>;fp1=fval1";
        Url url(szUrl);
        sprintf(msg, "%s", szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, "sipfoundry.org", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "autoattendant", getUserId(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "autoattendant@sipfoundry.org:5100",
            getIdentity(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 5100, url.getHostPort());
        ASSERT_STR_EQUAL_MESSAGE(msg, "http://localhost:8090/sipx-cgi/mediaserver.cgi?"
            "action=autoattendant", getParam("play", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "hval1", getHeaderParam("hp1", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "fval1", getFieldParam("fp1", url));

        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg,
                                 "sip:autoattendant@sipfoundry.org:5100;"
                                 "play=http%3A%2F%2Flocalhost%3A8090%2Fsipx-cgi%2Fmediaserver.cgi"
                                 "%3Faction%3Dautoattendant?hp1=hval1",
                                 getUri(url));
    }

    void testNoFieldParams()
    {
        const char *szUrl = "<sip:tester@sipfoundry.org;up1=uval1;up2=uval2?hp1=hval1&hp2=hval2>";
        Url url(szUrl);
        sprintf(msg, "%s", szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, "uval1", getParam("up1", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "uval2", getParam("up2", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "hval1", getHeaderParam("hp1", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "hval2", getHeaderParam("hp2", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, MISSING_PARAM, getFieldParam("up1", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, MISSING_PARAM, getFieldParam("hp1", url));

        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(
           msg,
           "sip:tester@sipfoundry.org;up1=uval1;up2=uval2?hp1=hval1&hp2=hval2",
           getUri(url)
                                 );
    }

    void testNoHeaderParams()
    {
        const char *szUrl = "Display Name<sip:tester@sipfoundry.org;up1=uval1;up2=uval2>"
            ";Fp1=fval1;Fp2=fval2";
        Url url(szUrl);
        sprintf(msg, "%s", szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, "uval1", getParam("up1", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "uval2", getParam("up2", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "fval1", getFieldParam("Fp1", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "fval2", getFieldParam("Fp2", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, MISSING_PARAM, getHeaderParam("up1", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, MISSING_PARAM, getHeaderParam("Fp1", url));

        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg,
                                 "sip:tester@sipfoundry.org;up1=uval1;up2=uval2",
                                 getUri(url));
    }

    void testCorrection()
    {
        const char *szUrl = "Display Name <Sip:tester@sipfoundry.org>";

        const char *szUrlCorrected = "Display Name<sip:tester@sipfoundry.org>";

        Url url(szUrl);
        sprintf(msg, "has space and wrong capitalization in Sip: %s", szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrlCorrected, toString(url));
    }

   void testSemiHeaderParam()
      {
         // ';' is reserved in header params, so it is represented '%3B'.
         const char *withAnglesEscaped   = "<sip:tester@sipfoundry.org?foo=bar%3Bbing>";
         const char *withoutAnglesEscaped = "sip:tester@sipfoundry.org?foo=bar%3Bbing";

         Url urlHdr(withAnglesEscaped);
         sprintf(msg, "with angle brackets %s", withAnglesEscaped);

         ASSERT_STR_EQUAL_MESSAGE(msg, "bar;bing", getHeaderParam("foo", urlHdr));

         ASSERT_STR_EQUAL_MESSAGE(msg, withAnglesEscaped, toString(urlHdr));
         ASSERT_STR_EQUAL_MESSAGE(msg, withoutAnglesEscaped, getUri(urlHdr));

         Url url(withoutAnglesEscaped, TRUE /* parse as a request uri */ );
         sprintf(msg, "without angle brackets %s", withoutAnglesEscaped);

         ASSERT_STR_EQUAL_MESSAGE(msg, "bar;bing", getHeaderParam("foo", url));

         ASSERT_STR_EQUAL_MESSAGE(msg, withAnglesEscaped, toString(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, withoutAnglesEscaped, getUri(url));
      }

   void testMissingAngles()
      {
         const char *szUrl = "sip:tester@sipfoundry.org?foo=bar";

         const char *szUrlCorrected = "<sip:tester@sipfoundry.org?foo=bar>";

         Url url(szUrl);
         sprintf(msg, "needed angle brackets %s", szUrl);
         ASSERT_STR_EQUAL_MESSAGE(msg, szUrlCorrected, toString(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "bar", getHeaderParam("foo", url));
      }

   void testNoAngleParam()
      {
         UtlString szUrl("sip:tester@sipfoundry.org;foo=bar");

         Url url(szUrl);
         sprintf(msg, "parameter without angle brackets %s", szUrl.data());
         ASSERT_STR_EQUAL_MESSAGE(msg, MISSING_PARAM, getParam("foo", url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "bar", getFieldParam("foo", url));

         Url requrl(szUrl, Url::AddrSpec);
         ASSERT_STR_EQUAL_MESSAGE(msg, "bar", getParam("foo", requrl));
         ASSERT_STR_EQUAL_MESSAGE(msg, MISSING_PARAM, getFieldParam("foo", requrl));
      }


    void testIpAddressOnly()
    {
        const char *szUrl = "10.1.1.225";
        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip:10.1.1.225", toString(url));
    }

   void testHostAddressOnly()
      {
         const char *szUrl = "somewhere.sipfoundry.org";
         Url url(szUrl);
         sprintf(msg, "host without scheme %s", szUrl);
         ASSERT_STR_EQUAL_MESSAGE(msg, "somewhere.sipfoundry.org", getHostAddress(url));
         ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip:somewhere.sipfoundry.org", toString(url));

         const char *szUrl2 = "some-where.sipfoundry.org";
         Url url2(szUrl2);
         ASSERT_STR_EQUAL_MESSAGE(msg, "some-where.sipfoundry.org", getHostAddress(url2));
         ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip:some-where.sipfoundry.org", toString(url2));

         UtlString hostWithPort;
         url.getHostWithPort(hostWithPort);
         ASSERT_STR_EQUAL("somewhere.sipfoundry.org", hostWithPort.data());
      }

   void testHostAndPort()
      {
         const char *szUrl = "somewhere.sipfoundry.org:333";
         Url url(szUrl);
         sprintf(msg, "hostport without scheme %s", szUrl);
         ASSERT_STR_EQUAL_MESSAGE(msg, "somewhere.sipfoundry.org", getHostAddress(url));
         CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 333, url.getHostPort());

         ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip:somewhere.sipfoundry.org:333", toString(url));

         UtlString hostWithPort;
         url.getHostWithPort(hostWithPort);
         ASSERT_STR_EQUAL("somewhere.sipfoundry.org:333", hostWithPort.data());
      }

    void testBogusPort()
    {
        const char *szUrl = "sip:1234@sipserver:abcd";
        Url url(szUrl);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, Url::UnknownUrlScheme, url.getScheme() );
    }

   void testMultiple()
      {
         const char *url1 =
            "Display Name<sip:uname@sipserver:555;"
            "tag=xxxxx;transport=TCP;msgId=4?call-Id=call2&cseq=2+INVITE>;"
            "fieldParam1=1234;fieldParam2=2345";
         const char *url2 =
            "Second Name<sip:two@sipserver2:666;"
            "tag=yyyy;transport=UCP;msgId=4?call-Id=call3&cseq=7+INVITE>;"
            "fieldParam1=5678;fieldParam2=6789";

         UtlString combinedUrl;
         combinedUrl.append(url1);
         combinedUrl.append(",");
         combinedUrl.append(url2);

         UtlString nextUrl;
         Url url(combinedUrl.data(), Url::NameAddr, &nextUrl);

         ASSERT_STR_EQUAL("sipserver", getHostAddress(url));
         ASSERT_STR_EQUAL("sip", getUrlType(url));
         ASSERT_STR_EQUAL("uname", getUserId(url));
         ASSERT_STR_EQUAL("Display Name", getDisplayName(url));
         CPPUNIT_ASSERT_EQUAL(555, url.getHostPort());
         ASSERT_STR_EQUAL("xxxxx", getParam("tag", url));
         ASSERT_STR_EQUAL("TCP", getParam("transport", url));
         ASSERT_STR_EQUAL("4", getParam("msgId", url));
         ASSERT_STR_EQUAL("call2", getHeaderParam("call-Id", url));
         ASSERT_STR_EQUAL("2 INVITE", getHeaderParam("cseq", url));
         ASSERT_STR_EQUAL("1234", getFieldParam("fieldParam1", url));
         ASSERT_STR_EQUAL("2345", getFieldParam("fieldParam2", url));

         ASSERT_STR_EQUAL(url1, toString(url));
         ASSERT_STR_EQUAL(
            "sip:uname@sipserver:555;tag=xxxxx;transport=TCP;msgId=4?call-Id=call2&cseq=2+INVITE",
            getUri(url)
                          );

         ASSERT_STR_EQUAL(url2, nextUrl.data());
      }

   void testIPv6Host()
      {
         const char *szUrl = "[a0:32:44::99]:333";
         Url url(szUrl);
         sprintf(msg, "IPv6 address %s", szUrl);
         ASSERT_STR_EQUAL_MESSAGE(msg, "[a0:32:44::99]", getHostAddress(url));
         CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 333, url.getHostPort());

         ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip:[a0:32:44::99]:333", toString(url));
      }


   void testNoBracketUrlWithAllParamsWithVaryingSpace()
    {
        const char *szUrl = "<sip:fsmith@sipfoundry.org:5555 ? call-id=12345 > ; "
            "msgId=5 ;msgId=6;transport=TCP";
        const char *szUrlCorrected = "<sip:fsmith@sipfoundry.org:5555?call-id=12345>;"
            "msgId=5;msgId=6;transport=TCP";

        Url url(szUrl);
        sprintf(msg, "%s", szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrlCorrected, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sipfoundry.org", getHostAddress(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 5555, url.getHostPort());
        ASSERT_STR_EQUAL_MESSAGE(msg, "fsmith", getUserId(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "12345", getHeaderParam("call-Id", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "5", getFieldParam("msgId", url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "5", getFieldParam("msgId", url, 0));
        ASSERT_STR_EQUAL_MESSAGE(msg, "6", getFieldParam("msgId", url, 1));
        ASSERT_STR_EQUAL_MESSAGE(msg, "TCP", getFieldParam("transport", url));
    }

    void testConstruction()
    {
        Url url;
        url.setUrlType("sip");
        url.setUserId("someuser") ;
        url.setHostAddress("main.sip") ;
        ASSERT_STR_EQUAL("sip:someuser@main.sip", toString(url));
        url.includeAngleBrackets() ;
        ASSERT_STR_EQUAL("<sip:someuser@main.sip>", toString(url));
    }

    void testHttpConstruction()
    {
        Url url;
        ASSERT_STR_EQUAL("sip:", toString(url));
        url.setUrlType("http") ;
        url.setHostAddress("web.server") ;
        url.setPath("/somewhere/in/cyber") ;
        url.setHostPort(8080) ;
        ASSERT_STR_EQUAL("http://web.server:8080/somewhere/in/cyber",
            toString(url));
    }

    void testComplexConstruction()
    {
        Url url;

        // Type should be set to sip by default. Verify that by not setting
        // anything for the type
        ASSERT_STR_EQUAL("sip:", toString(url));

        url.setUserId("raghu");
        url.setPassword("rgpwd");
        url.setHostAddress("sf.org");
        ASSERT_STR_EQUAL("sip:raghu:rgpwd@sf.org", toString(url));

        url.setUrlParameter("up1", "uval1");
        url.setUrlParameter("up2", "uval2");
        ASSERT_STR_EQUAL("<sip:raghu:rgpwd@sf.org;up1=uval1;up2=uval2>", toString(url));

        url.setHeaderParameter("hp1", "hval1");
        url.setHeaderParameter("hp2", "hval2");
        ASSERT_STR_EQUAL("<sip:raghu:rgpwd@sf.org;up1=uval1;up2=uval2?hp1=hval1&hp2=hval2>",
                         toString(url));

        url.setFieldParameter("fp1", "fval1");
        ASSERT_STR_EQUAL("<sip:raghu:rgpwd@sf.org;"
                         "up1=uval1;up2=uval2?hp1=hval1&hp2=hval2>;fp1=fval1", toString(url));

        url.setDisplayName("Raghu Venkataramana");
        ASSERT_STR_EQUAL("Raghu Venkataramana<sip:raghu:rgpwd@sf.org;"
                         "up1=uval1;up2=uval2?hp1=hval1&hp2=hval2>;fp1=fval1", toString(url));
    }

    void testAddAttributesToExisting()
    {
        Url url("sip:u@host");

        url.setDisplayName("New Name");
        ASSERT_STR_EQUAL("New Name<sip:u@host>", toString(url));

        url.setHostPort(5070);
        ASSERT_STR_EQUAL("New Name<sip:u@host:5070>", toString(url));

        url.setUrlParameter("u1", "uv1");
        ASSERT_STR_EQUAL("New Name<sip:u@host:5070;u1=uv1>", toString(url));

        url.setHeaderParameter("h1", "hv1");
        ASSERT_STR_EQUAL("New Name<sip:u@host:5070;u1=uv1?h1=hv1>", toString(url));

        url.setFieldParameter("f1", "fv1");
        ASSERT_STR_EQUAL("New Name<sip:u@host:5070;u1=uv1?h1=hv1>;f1=fv1", toString(url));

        Url url2("sip:u@host");

        url2.setFieldParameter("field", "value2");
        ASSERT_STR_EQUAL("<sip:u@host>;field=value2", toString(url2));

    }

    void testComplexDisplayName()
    {
       UtlString s;
       bool b;

       // The case that first revealed the problem with parsing display names.

       Url url("\"Massimo Vignone\" <sip:8032@192.168.3.2:5060>;expires=3600;+sip.instance=\"<00000000-0000-0000-0000-000E08DEEEC6>\"",
               Url::NameAddr,
               NULL);

       url.getDisplayName(s);
       ASSERT_STR_EQUAL("\"Massimo Vignone\"", s.data());
       url.getFieldParameter(SIP_EXPIRES_FIELD, s);
       ASSERT_STR_EQUAL("3600", s.data());
       url.getFieldParameter("+sip.instance", s);
       ASSERT_STR_EQUAL("<00000000-0000-0000-0000-000E08DEEEC6>", s.data());

       url.setFieldParameter(SIP_EXPIRES_FIELD, "4810");
       ASSERT_STR_EQUAL("\"Massimo Vignone\"<sip:8032@192.168.3.2:5060>;expires=4810;+sip.instance=\"<00000000-0000-0000-0000-000E08DEEEC6>\"", toString(url));

       // A number of messy display names.
       struct {
          const char* url;
          const char* expected;
       } tests[] = {
          { "\"Massimo Vignone\" <sip:8032@192.168.3.2:5060>",
            "\"Massimo Vignone\""},
          // Test the handling of embeded backslashes and double-quotes.
          // Be careful of the quoting -- the characters in this URI are
          // backslash-doublequote.
          { "\"Massimo\\\" Vignone\" <sip:8032@192.168.3.2:5060>",
            "\"Massimo\\\" Vignone\""},
          { "\"Massimo\\\\ Vignone\" <sip:8032@192.168.3.2:5060>",
            "\"Massimo\\\\ Vignone\""},
          { "\"Massimo\\\"\\\\\\\" Vignone\" <sip:8032@192.168.3.2:5060>",
            "\"Massimo\\\"\\\\\\\" Vignone\""},
          { "\"Massimo Vignone\\\\\" <sip:8032@192.168.3.2:5060>",
            "\"Massimo Vignone\\\\\""},
          { "\"Massimo\\\" \\\"Vignone\" <sip:8032@192.168.3.2:5060>",
            "\"Massimo\\\" \\\"Vignone\""},
          // You can backslash-quote any ordinary character.
          { "\"\\M\" <sip:8032@192.168.3.2:5060>",
            "\"\\M\""},
          { "\"\\M\\a\\s\\s\\i\\m\\o\\ \\V\\i\\g\\n\\o\\n\\e\" <sip:8032@192.168.3.2:5060>",
            "\"\\M\\a\\s\\s\\i\\m\\o\\ \\V\\i\\g\\n\\o\\n\\e\""},
          // Handling of Latin-1 characters in UTF-8 encoding.
          // a+grave = U+00E0 = UTF-8 C3:A0
          { "\"Ufficio Attivit\xC3\xA0 Grafiche\" <sip:8032@192.168.3.2:5060>",
            "\"Ufficio Attivit\xC3\xA0 Grafiche\""},
          // Handling of Cyrilic in UTF-8 encoding
          { "\"\321\202\320\265\320\273\320\265 C\320\275\320\276\320\274_3606\" <sip:3606@sip.svgc.ru>;tag=d79q2a40i3",
            "\"\321\202\320\265\320\273\320\265 C\320\275\320\276\320\274_3606\""},
       };

       for (unsigned int i = 0; i < sizeof (tests) / sizeof (tests[0]); i++)
       {
          sprintf(msg, "URI %d: %s", i, tests[i].url);
          Url url;
          b = url.fromString(tests[i].url, Url::NameAddr, NULL);
          CPPUNIT_ASSERT_MESSAGE(msg, b);
          url.getDisplayName(s);
          ASSERT_STR_EQUAL_MESSAGE(msg, tests[i].expected, s.data());
       }

       // Check strings that should cause parsing to fail.

       const char* fail_tests[] = {
          "\"xxxx <sip:8032@192.168.3.2:5060>",
          "|||<sip:8032@192.168.3.2:5060>",
          "<|||sip:8032@192.168.3.2:5060>",
          "<sip:|||8032@192.168.3.2:5060>",
          "<sip:8032@192.168.3.2:5060|||;foo=bar>",
          "<sip:8032@192.168.3.2:5060|||?foo=bar>",
          "<sip:8032@192.168.3.2:5060;foo=bar,>",
          "<sip:8032@192.168.3.2:5060?foo=bar,>",
          "<sip:600-3@cdhcp139.pingtel.com&q=0.8>",
       };

       for (unsigned int i = 0;
            i < sizeof (fail_tests) / sizeof (fail_tests[0]);
            i++)
       {
          sprintf(msg, "URI %d: %s", i, fail_tests[i]);
          Url url;
          b = url.fromString(fail_tests[i], Url::NameAddr, NULL);
          CPPUNIT_ASSERT_MESSAGE(msg, !b);
       }
    }

    void testChangeValues()
    {
        Url url("New Name<sip:u@host:5070;u1=uv1?h1=hv1>;f1=fv1");

        ASSERT_STR_EQUAL("New Name<sip:u@host:5070;u1=uv1?h1=hv1>;f1=fv1",
                         toString(url));

        // Only the changed attributes should actually change.

        url.setDisplayName("Changed Name");
        ASSERT_STR_EQUAL("Changed Name<sip:u@host:5070;u1=uv1?h1=hv1>;f1=fv1",
                         toString(url));

        url.setHostPort(PORT_NONE);
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1?h1=hv1>;f1=fv1",
                         toString(url));

        url.setHeaderParameter("h1", "hv2");
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1?h1=hv1&h1=hv2>;f1=fv1",
                         toString(url));

        url.setHeaderParameter("expires", "10");
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1?h1=hv1&h1=hv2&expires=10>;f1=fv1",
                         toString(url));

        url.setHeaderParameter("expires", "20");
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1?h1=hv1&h1=hv2&expires=20>;f1=fv1",
                         toString(url));

        url.setFieldParameter("f2", "fv2");
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1?h1=hv1&h1=hv2&expires=20>;f1=fv1;f2=fv2",
                         toString(url));

        url.setHeaderParameter("route", "rt1");
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1?h1=hv1&h1=hv2&expires=20&route=rt1>;f1=fv1;f2=fv2",
                         toString(url));

        url.setHeaderParameter("ROUTE", "rt2,rt1");
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1?h1=hv1&h1=hv2&expires=20&ROUTE=rt2%2Crt1>;f1=fv1;f2=fv2",
                         toString(url));
    }

    void testRemoveAttributes()
    {
        Url url("Changed Name<sip:u@host;u1=uv1;u2=uv2"
                "?h1=hv1&h2=hnew2>;f1=fv1;f2=fv2");
        url.removeHeaderParameter("h1") ;
        url.removeFieldParameter("f1") ;
        url.removeUrlParameter("u1") ;

        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u2=uv2?h2=hnew2>;f2=fv2",
            toString(url));
        ASSERT_STR_EQUAL(MISSING_PARAM, getHeaderParam("h1", url));
        ASSERT_STR_EQUAL(MISSING_PARAM, getFieldParam("f1", url));
        ASSERT_STR_EQUAL(MISSING_PARAM, getParam("u1", url));
    }

    // Test that removeUrlParameter is case-insensitive in parameter names.
    void testRemoveUrlParameterCase()
    {
       Url url0("<sip:600-3@cdhcp139;q=0.8>");
       url0.removeUrlParameter("q");

       ASSERT_STR_EQUAL("sip:600-3@cdhcp139",
                        toString(url0));

       Url url1("<sip:600-3@cdhcp139.pingtel.com;q=0.8>");
       url1.removeUrlParameter("q");

       ASSERT_STR_EQUAL("sip:600-3@cdhcp139.pingtel.com",
                        toString(url1));

        Url url2("<sip:600-3@cdhcp139.pingtel.com;q=0.8;z=q>");
        url2.removeUrlParameter("Q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com;z=q>",
                         toString(url2));

        Url url3("<sip:600-3@cdhcp139.pingtel.com;abcd=27;Q=0.8>");
        url3.removeUrlParameter("q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com;abcd=27>",
                         toString(url3));

        Url url4("<sip:600-3@cdhcp139.pingtel.com;Q=0.8;CXV>");
        url4.removeUrlParameter("Q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com;CXV>",
                         toString(url4));

        Url url5("<sip:600-3@cdhcp139.pingtel.com;mIxEdCaSe=0.8>");
        url5.removeUrlParameter("MiXeDcAsE");

        ASSERT_STR_EQUAL("sip:600-3@cdhcp139.pingtel.com",
                         toString(url5));
    }

    // Test that removeFieldParameter is case-insensitive in parameter names.
    void testRemoveFieldParameterCase()
    {
        Url url1("<sip:600-3@cdhcp139.pingtel.com>;q=0.8");
        url1.removeFieldParameter("q");

        ASSERT_STR_EQUAL("sip:600-3@cdhcp139.pingtel.com",
                         toString(url1));

        Url url2("<sip:600-3@cdhcp139.pingtel.com>;q=0.8;z=q");
        url2.removeFieldParameter("Q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com>;z=q",
                         toString(url2));

        Url url3("<sip:600-3@cdhcp139.pingtel.com>;abcd=27;Q=0.8");
        url3.removeFieldParameter("q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com>;abcd=27",
                         toString(url3));

        Url url4("<sip:600-3@cdhcp139.pingtel.com>;Q=0.8;CXV");
        url4.removeFieldParameter("Q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com>;CXV",
                         toString(url4));

        Url url5("<sip:600-3@cdhcp139.pingtel.com>;mIxEdCaSe=0.8");
        url5.removeFieldParameter("MiXeDcAsE");

        ASSERT_STR_EQUAL("sip:600-3@cdhcp139.pingtel.com",
                         toString(url5));
    }

    // Test that removeHeaderParameter is case-insensitive in parameter names.
    void testRemoveHeaderParameterCase()
    {
        Url url1("<sip:600-3@cdhcp139.pingtel.com?q=0.8>");
        url1.removeHeaderParameter("q");

        ASSERT_STR_EQUAL("sip:600-3@cdhcp139.pingtel.com",
                         toString(url1));

        Url url2("<sip:600-3@cdhcp139.pingtel.com?q=0.8&z=q>");
        url2.removeHeaderParameter("Q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com?z=q>",
                         toString(url2));

        Url url3("<sip:600-3@cdhcp139.pingtel.com?abcd=27&Q=0.8>");
        url3.removeHeaderParameter("q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com?abcd=27>",
                         toString(url3));

        Url url5("<sip:600-3@cdhcp139.pingtel.com?mIxEdCaSe=0.8>");
        url5.removeHeaderParameter("MiXeDcAsE");

        ASSERT_STR_EQUAL("sip:600-3@cdhcp139.pingtel.com",
                         toString(url5));
    }

    void testRemoveAllTypesOfAttributes()
    {
        const char* szUrl = "Changed Name<sip:u@host;u1=uv1;u2=uv2"
            "?h1=hv1&h2=hchanged2&h3=hnew3>;f1=fv1;f2=fv2";

        Url noHeader(szUrl);
        noHeader.removeHeaderParameters();
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1;u2=uv2>;f1=fv1;f2=fv2",
            toString(noHeader));

        Url noUrl(szUrl);
        noUrl.removeUrlParameters();
        ASSERT_STR_EQUAL("Changed Name<sip:u@host?h1=hv1&h2=hchanged2&h3=hnew3>;f1=fv1;f2=fv2",
            toString(noUrl));

        Url noField(szUrl);
        noField.removeFieldParameters();
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1;u2=uv2?h1=hv1&h2=hchanged2&h3=hnew3>",
            toString(noField));

        Url noParameters(szUrl);
        noParameters.removeParameters();
        ASSERT_STR_EQUAL("Changed Name<sip:u@host>",
            toString(noParameters));
    }

    void testRemoveAngleBrackets()
    {
        Url url("<sip:u@host:5070>") ;
        url.removeAngleBrackets() ;
        ASSERT_STR_EQUAL("sip:u@host:5070", toString(url));
    }

    void testReset()
    {
        Url url("Changed Name<sip:u@host;u1=uv1;u2=uv2?h1=hv1"
                "&h2=hchanged2&h3=hnew3>;f1=fv1;f2=fv2") ;
        url.reset();
        ASSERT_STR_EQUAL("sip:", toString(url));
    }

    void testAssignment()
    {
        const char* szUrl = "Raghu Venkataramana<sip:raghu:rgpwd@sf.org;"
                "up1=uval1;up2=uval2?hp1=hval1&hp2=hval2>;fp1=fval1";
        Url equalsSz = szUrl;
        ASSERT_STR_EQUAL(szUrl, toString(equalsSz));

        Url equalsUrl = Url(szUrl);
        ASSERT_STR_EQUAL(szUrl, toString(equalsUrl));
    }

    void testGetAllParameters()
    {
        UtlString paramNames[16];
        UtlString paramValues[16];
        int paramCount = 0;

        const char* szUrl = "<sip:1234@ss.org;u1=uv1;u2=uv2;u3=uv3?h1=hv1&h2=hv2>;"
            "f1=fv1;f2=fv2;f3=fv3";

        // URL params
        Url url(szUrl);
        sprintf(msg, "Test false when invalid arguments %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.getUrlParameters(0, NULL, NULL, paramCount));

        sprintf(msg, "Test true when valid arguments %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(szUrl, url.getUrlParameters(3, paramNames,
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, 3, paramCount);

        sprintf(msg, "Test valid arguments %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "u1 u2 u3", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "uv1 uv2 uv3", paramValues);

        // Header params
        sprintf(msg, "valid header param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.getHeaderParameters(2, paramNames,
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 2, paramCount);

        sprintf(msg, "header params  %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "h1 h2", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "hv1 hv2", paramValues);

        // Field params
        sprintf(msg, "valid field param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.getFieldParameters(3, paramNames,
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 3, paramCount);

        sprintf(msg, "field params  %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "f1 f2 f3", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "fv1 fv2 fv3", paramValues);
    }

    void testGetDuplicateNamedParameters()
    {
        UtlString paramNames[16];
        UtlString paramValues[16];
        int paramCount = 0;

        const char* szUrl = "D Name<sip:abc@server:5050;p1=u1;p2=u2;p1=u3?"
            "p1=h1&p2=h2>;p1=f1;p2=f2";

        // URL params
        Url url(szUrl);
        sprintf(msg, "Test true when valid arguments %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(szUrl, url.getUrlParameters(3, paramNames,
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, 3, paramCount);

        sprintf(msg, "Test valid arguments %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "p1 p2 p1", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "u1 u2 u3", paramValues);

        // Header params
        sprintf(msg, "valid header param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.getHeaderParameters(2, paramNames,
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 2, paramCount);

        sprintf(msg, "header params  %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "p1 p2", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "h1 h2", paramValues);

        // Field params
        sprintf(msg, "valid field param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.getFieldParameters(2, paramNames,
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 2, paramCount);

        sprintf(msg, "field params  %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "p1 p2", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "f1 f2", paramValues);
    }

    void testGetOnlyUrlParameters()
    {
        UtlString paramNames[16];
        UtlString paramValues[16];
        int paramCount = 0;

        const char* szUrl = "D Name<sip:abc@server;up1=u1;up2=u2>";
        // URL params
        Url url(szUrl);
        sprintf(msg, "Test true when valid arguments %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(szUrl, url.getUrlParameters(2, paramNames,
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, 2, paramCount);

        sprintf(msg, "Test valid arguments %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "up1 up2", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "u1 u2", paramValues);

        // Header params
        sprintf(msg, "valid header param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.getHeaderParameters(10, paramNames,
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 0, paramCount);

        // Field params
        sprintf(msg, "valid field param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.getFieldParameters(10, paramNames,
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 0, paramCount);
    }

    void testGetOnlyHeaderParameters()
    {
        UtlString paramNames[16];
        UtlString paramValues[16];
        int paramCount = 0;

        const char* szUrl = "D Name<sip:abc@server?h1=hv1&h2=hv2>";
        // URL params
        Url url(szUrl);
        sprintf(msg, "Test true when valid arguments %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(szUrl, !url.getUrlParameters(10, paramNames,
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, 0, paramCount);

        // Header params
        sprintf(msg, "valid header param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.getHeaderParameters(2, paramNames,
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 2, paramCount);

        sprintf(msg, "Test valid arguments %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "h1 h2", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "hv1 hv2", paramValues);

        // Field params
        sprintf(msg, "valid field param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.getFieldParameters(10, paramNames,
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 0, paramCount);
    }

    void testGetOnlyFieldParameters()
    {
        UtlString paramNames[16];
        UtlString paramValues[16];
        int paramCount = 0;

        const char* szUrl = "D Name<sip:abc@server>;f1=fv1;f2=fv2";
        // URL params
        Url url(szUrl);
        sprintf(msg, "Test true when valid arguments %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(szUrl, !url.getUrlParameters(10, paramNames,
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, 0, paramCount);

        // Header params
        sprintf(msg, "valid header param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.getHeaderParameters(10, paramNames,
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 0, paramCount);

        // Field params
        sprintf(msg, "valid field param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.getFieldParameters(2, paramNames,
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 2, paramCount);

        sprintf(msg, "Test valid arguments %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "f1 f2", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "fv1 fv2", paramValues);
    }

    // Test that getUrlParameter is case-insensitive in parameter names.
    void testGetUrlParameterCase()
    {
       UtlString value;

       Url url1("<sip:600-3@cdhcp139.pingtel.com;q=0.8>");

       url1.getUrlParameter("q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url2("<sip:600-3@cdhcp139.pingtel.com;q=0.8;z=q>");

       url2.getUrlParameter("Q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url3("<sip:600-3@cdhcp139.pingtel.com;abcd=27;Q=0.8>");

       url3.getUrlParameter("q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url4("<sip:600-3@cdhcp139.pingtel.com;Q=0.8;CXV>");

       url4.getUrlParameter("Q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url5("<sip:600-3@cdhcp139.pingtel.com;mIxEdCaSe=0.8>");

       url5.getUrlParameter("MiXeDcAsE", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       // Fetch instances of a paramerter that is present multiple times.
       // (Though multiple presences aren't allowed by RFC 3261 sec. 19.1.1.)

       Url url6("<sip:600-3@cdhcp139.pingtel.com;abcd=27;Q=0.8;Abcd=\"12\";ABCD=EfG>");

       url6.getUrlParameter("abcd", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getUrlParameter("abcd", value, 1);
       // Double-quotes do not quote URI parameters.
       ASSERT_STR_EQUAL("\"12\"", value.data());
       url6.getUrlParameter("abcd", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());

       url6.getUrlParameter("ABCD", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getUrlParameter("ABCD", value, 1);
       // Double-quotes do not quote URI parameters.
       ASSERT_STR_EQUAL("\"12\"", value.data());
       url6.getUrlParameter("ABCD", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());

       url6.getUrlParameter("aBCd", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getUrlParameter("aBCd", value, 1);
       // Double-quotes do not quote URI parameters.
       ASSERT_STR_EQUAL("\"12\"", value.data());
       url6.getUrlParameter("aBCd", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());
    }

    // Test that getFieldParameter is case-insensitive in parameter names.
    void testGetFieldParameterCase()
    {
       UtlString value;

       Url url00("<sip:333@212.247.206.174:2052;transport=tcp;line=98tq8dsn>;q=1.0;+sip.instance=\"<urn:uuid:1d960183-88c9-4813-80e1-b97946c09465>\";audio;mobility=\"fixed\";duplex=\"full\";description=\"snom320\";actor=\"principal\";events=\"dialog\";methods=\"INVITE,ACK,CANCEL,BYE,REFER,OPTIONS,NOTIFY,SUBSCRIBE,PRACK,MESSAGE,INFO\"");

       url00.getFieldParameter("+sip.instance", value, 0);

       ASSERT_STR_EQUAL("<urn:uuid:1d960183-88c9-4813-80e1-b97946c09465>",
                        value.data());

       Url url1("<sip:600-3@cdhcp139.pingtel.com>;q=0.8");

       url1.getFieldParameter("q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url2("<sip:600-3@cdhcp139.pingtel.com>;q=0.8;z=q");

       url2.getFieldParameter("Q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url3("<sip:600-3@cdhcp139.pingtel.com>;abcd=27;Q=0.8");

       url3.getFieldParameter("q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url4("<sip:600-3@cdhcp139.pingtel.com>;Q=0.8;CXV");

       url4.getFieldParameter("Q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url5("<sip:600-3@cdhcp139.pingtel.com>;mIxEdCaSe=0.8");

       url5.getFieldParameter("MiXeDcAsE", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       // Fetch instances of a paramerter that is present multiple times.
       // (Though multiple presences aren't allowed by RFC 3261 sec. 19.1.1.)

       Url url6("<sip:600-3@cdhcp139.pingtel.com>;abcd=27;Q=0.8;Abcd=\"12\";ABCD=EfG");

       url6.getFieldParameter("abcd", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getFieldParameter("abcd", value, 1);
       // getFieldParameter now de-quotes field values.
       ASSERT_STR_EQUAL("12", value.data());
       url6.getFieldParameter("abcd", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());

       url6.getFieldParameter("ABCD", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getFieldParameter("ABCD", value, 1);
       // getFieldParameter now un-quotes quoted values.
       ASSERT_STR_EQUAL("12", value.data());
       url6.getFieldParameter("ABCD", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());

       url6.getFieldParameter("aBCd", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getFieldParameter("aBCd", value, 1);
       // getFieldParameter now de-quotes field values.
       ASSERT_STR_EQUAL("12", value.data());
       url6.getFieldParameter("aBCd", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());
    }

    // Test that getHeaderParameter is case-insensitive in parameter names.
    void testGetHeaderParameterCase()
    {
       UtlString value;

       Url url1("<sip:600-3@cdhcp139.pingtel.com?q=0.8>");

       url1.getHeaderParameter("q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url2("<sip:600-3@cdhcp139.pingtel.com?q=0.8&z=q>");

       url2.getHeaderParameter("Q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url3("<sip:600-3@cdhcp139.pingtel.com?abcd=27&Q=0.8>");

       url3.getHeaderParameter("q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url5("<sip:600-3@cdhcp139.pingtel.com?mIxEdCaSe=0.8>");

       url5.getHeaderParameter("MiXeDcAsE", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       // Fetch instances of a header that is present multiple times.

       Url url6("<sip:600-3@cdhcp139.pingtel.com?abcd=27&Q=0.8&Abcd=12&ABCD=EfG>");

       url6.getHeaderParameter("abcd", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getHeaderParameter("abcd", value, 1);
       ASSERT_STR_EQUAL("12", value.data());
       url6.getHeaderParameter("abcd", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());

       url6.getHeaderParameter("ABCD", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getHeaderParameter("ABCD", value, 1);
       ASSERT_STR_EQUAL("12", value.data());
       url6.getHeaderParameter("ABCD", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());

       url6.getHeaderParameter("aBCd", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getHeaderParameter("aBCd", value, 1);
       ASSERT_STR_EQUAL("12", value.data());
       url6.getHeaderParameter("aBCd", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());
    }

    void testIsDigitString()
    {
        CPPUNIT_ASSERT_MESSAGE("Verify isDigitString for a single digit",
            Url::isDigitString("1")) ;

        CPPUNIT_ASSERT_MESSAGE("Verify isDigitString for a long numeric string",
            Url::isDigitString("1234567890234586")) ;

        CPPUNIT_ASSERT_MESSAGE("Verify isDigitString returns false for an alpha string",
            !Url::isDigitString("abcd")) ;

        CPPUNIT_ASSERT_MESSAGE("Verify isDigitString returns false for alpha then pound key",
            !Url::isDigitString("1234#")) ;

        CPPUNIT_ASSERT_MESSAGE("Verify isDigitString for a string that has a star key in it",
            Url::isDigitString("*6")) ;
    }

    void testIsUserHostPortEqualExact()
    {
        const char *szUrl = "Raghu Venkatarmana<sip:raghu@sf.org:5080;blah=b?bl=a>;bdfdf=ere";
        const char *szTest                       = "raghu@sf.org:5080";
        Url url(szUrl);

        sprintf(msg, "test=%s, url=%s", szTest, szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.isUserHostPortEqual(szTest));
    }

    void testIsUserHostPortVaryCapitalization()
    {
        const char *szUrl = "R V<sip:raghu@SF.oRg:5080>";
        const char *szTest =        "raghu@sf.org:5080";
        Url url(szUrl);

        sprintf(msg, "test=%s, url=%s", szTest, szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.isUserHostPortEqual(szTest));
    }

    void testIsUserHostPortNoPort()
    {
        const char *szUrl = "R V<sip:raghu@sf.org>";
        const char *szTest =        "raghu@sf.org:5060";
        Url url(szUrl);

        sprintf(msg, "test=%s, url=%s", szTest, szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.isUserHostPortEqual(szTest));
        CPPUNIT_ASSERT_MESSAGE(msg, url.isUserHostPortEqual(szTest, 5060));
    }

    void testIsUserHostPortNoMatch()
    {
        const char *szUrl = "R V<sip:Raghu@SF.oRg:5080>";
        const char *szTest =        "raghu@sf.org:5080";
        Url url(szUrl);

        sprintf(msg, "test=%s, url=%s", szTest, szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.isUserHostPortEqual(szTest));
    }

    void testIsUserHostPortPorts()
    {
       // Urls which are all the same.
       {
          const char* strings[] = {
             "R V<sip:Raghu@SF.oRg:5080>",
             "<sip:Raghu@SF.oRg:5080>",
             "Raghu@SF.oRg:5080",
             // Make sure that case differences are handled correctly.
             "Raghu@sf.org:5080",
          };
          Url urls[sizeof (strings) / sizeof (strings[0])];
          unsigned int i;

          // Set up the Url objects.
          for (i = 0; i < sizeof (strings) / sizeof (strings[0]);
               i++)
          {
             urls[i] = strings[i];
          }

          // Do all the comparisons.
          for (i = 0; i < sizeof (strings) / sizeof (strings[0]);
               i++)
          {
             for (unsigned int j = 0;
                  j < sizeof (strings) / sizeof (strings[0]);
                  j++)
             {
                int expected = TRUE;
                int actual = urls[i].isUserHostPortEqual(urls[j]);
                char msg[80];
                sprintf(msg, "%s != %s", strings[i], strings[j]);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, expected, actual);
             }
          }
       }

       // Urls which are all different.
       {
          const char* strings[] = {
             "sip:foo@bar",
             "sip:foo@bar:5060",
             "sip:foo@bar:1",
             "sip:foo@bar:100",
             "sip:foo@bar:65535",
             // Make sure case differences are detected.
             "sip:Foo@bar",
          };
          Url urls[sizeof (strings) / sizeof (strings[0])];
          unsigned int i;

          // Set up the Url objects.
          for (i = 0; i < sizeof (strings) / sizeof (strings[0]);
               i++)
          {
             urls[i] = strings[i];
          }

          // Do all the comparisons.
          for (i = 0; i < sizeof (strings) / sizeof (strings[0]);
               i++)
          {
             for (unsigned int j = 0;
                  j < sizeof (strings) / sizeof (strings[0]);
                  j++)
             {
                int expected = (i == j);
                int actual = urls[i].isUserHostPortEqual(urls[j]);
                char msg[80];
                sprintf(msg, "%s == %s", strings[i], strings[j]);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, expected, actual);
             }
          }
       }
    }

    void testToString()
    {
        const char* szUrl = "sip:192.168.1.102" ;
        Url url(szUrl) ;
        UtlString toString("SHOULD_BE_REPLACED");
        url.toString(toString) ;

        // Verify that toString replaces (as opposed to append)
        ASSERT_STR_EQUAL(szUrl, toString.data()) ;
    }

   void testFromString()
      {
         Url url;

         UtlString inputUrl("sip:192.168.1.102");
         CPPUNIT_ASSERT(url.fromString(inputUrl, Url::NameAddr));

         UtlString firstString("SHOULD_BE_REPLACED");
         url.toString(firstString) ;

         ASSERT_STR_EQUAL(inputUrl.data(), firstString.data()) ;

         UtlString rewrittenUrl("sip:user@host");
         CPPUNIT_ASSERT(url.fromString(rewrittenUrl, Url::AddrSpec));

         UtlString secondString("SHOULD_BE_REPLACED");
         url.toString(secondString) ;

         ASSERT_STR_EQUAL(rewrittenUrl.data(), secondString.data()) ;

         UtlString badUrl("!bad:");
         CPPUNIT_ASSERT(!url.fromString(badUrl));
         CPPUNIT_ASSERT_EQUAL(Url::UnknownUrlScheme, url.getScheme());
      }

    void testGetIdentity()
    {
       // The data for a single test.
       struct test {
          // The string to convert to a URI.
          const char* string;
          // The identity to be returned by getIdentity().
          const char* identity;
       };
       // The tests.
       struct test tests[] = {
          { "sip:foo@bar", "foo@bar" },
          { "sip:foo@bar:5060", "foo@bar:5060" },
          { "sip:foo@bar:1", "foo@bar:1" },
          { "sip:foo@bar:100", "foo@bar:100" },
          { "sip:foo@bar:65535", "foo@bar:65535" },
          { "<sip:foo@bar>", "foo@bar" },
          { "<sip:foo@bar:5060>", "foo@bar:5060" },
          { "<sip:foo@bar:1>", "foo@bar:1" },
          { "<sip:foo@bar:100>", "foo@bar:100" },
          { "<sip:foo@bar:65535>", "foo@bar:65535" },
       };

       for (unsigned int i = 0; i < sizeof (tests) / sizeof (tests[0]);
            i++)
       {
          Url url = tests[i].string;
          ASSERT_STR_EQUAL_MESSAGE(tests[i].string, tests[i].identity,
                                   getIdentity(url));
       }
    }

/*
 * The primary purpose of the following tests is to look for cases in which
 * the regular expressions either take too long to run, or recurse too deeply
 * (the latter will usually also cause the former).
 *
 * Because we don't want the tests to be chatty, and we don't want to pick an
 * arbitrary time for "too long", we use the following PARSE macro.  It wraps the
 * constructor and in the verbose form prints how long it took to run.
 * Normally, this should be disabled, but turn it on if you're making changes
 * to the regular expressions so that you can see any performance/recursion
 * problems.
 */
#if LOG_PARSE_TIME
#     define PARSE(name)                        \
      OsTimeLog name##timeLog;                  \
      name##timeLog.addEvent("start  " #name);  \
      Url name##Url(name);                      \
      name##timeLog.addEvent("parsed " #name);  \
      UtlString name##Log;                      \
      name##timeLog.getLogString(name##Log);    \
      printf("\n%s\n", name##Log.data());
#else
#     define PARSE(name)                        \
      Url name##Url(name);
#endif

   void testNormalUri()
      {
         // exists just to provide a time comparison for the following testBigUri... tests
         UtlString normal("Display Name <sip:user@example.com>");
         PARSE(normal);
      }

   void testBigUriDisplayName()
      {
         // <bigtoken> sip:user@example.com

         UtlString bigname;
         bigname.append(bigtoken);
         bigname.append(" <sip:user@example.com>");

         PARSE(bigname);

         CPPUNIT_ASSERT_EQUAL(Url::SipUrlScheme, bignameUrl.getScheme());

         UtlString component;

         bignameUrl.getDisplayName(component);
         CPPUNIT_ASSERT(! component.compareTo(bigtoken));

         bignameUrl.getUserId(component);
         CPPUNIT_ASSERT(! component.compareTo("user"));

         bignameUrl.getHostAddress(component);
         CPPUNIT_ASSERT(! component.compareTo("example.com"));
      }

   void testBigUriQuotedName()
      {
         // "<bigtoken>" sip:user@example.com

         UtlString bigquotname;
         bigquotname.append("\"");
         bigquotname.append(bigtoken);
         bigquotname.append("\" <sip:user@example.com>");

         PARSE(bigquotname);

         UtlString component;

         bigquotnameUrl.getDisplayName(component);
         UtlString quoted_bigtoken;
         quoted_bigtoken.append("\"");
         quoted_bigtoken.append(bigtoken);
         quoted_bigtoken.append("\"");
         CPPUNIT_ASSERT(! component.compareTo(quoted_bigtoken));

         CPPUNIT_ASSERT_EQUAL(Url::SipUrlScheme, bigquotnameUrl.getScheme());

         bigquotnameUrl.getUserId(component);
         CPPUNIT_ASSERT(! component.compareTo("user"));

         bigquotnameUrl.getHostAddress(component);
         CPPUNIT_ASSERT(! component.compareTo("example.com"));
      }

   void testBigUriScheme()
      {
	 // Unknown scheme <bigtoken> is taken to be the user, with an implicit sip:.
         // <bigtoken>:password@example.com

         UtlString bigscheme;
         bigscheme.append(bigtoken);
         bigscheme.append(":password@example.com");

         PARSE(bigscheme);

         UtlString component;

         CPPUNIT_ASSERT_EQUAL(Url::SipUrlScheme, bigschemeUrl.getScheme());

         bigschemeUrl.getUserId(component);
         ASSERT_STR_EQUAL(bigtoken, component.data());

         bigschemeUrl.getPassword(component);
         ASSERT_STR_EQUAL("password", component.data());

         bigschemeUrl.getHostAddress(component);
         ASSERT_STR_EQUAL("example.com", component.data());

         Url bigSchemeAddrSpec(bigscheme, TRUE /* as addr-spec */);

         CPPUNIT_ASSERT_EQUAL(Url::SipUrlScheme, bigschemeUrl.getScheme());

         bigSchemeAddrSpec.getUserId(component);
         ASSERT_STR_EQUAL(bigtoken, component.data());

         bigSchemeAddrSpec.getPassword(component);
         ASSERT_STR_EQUAL("password", component.data());

         bigSchemeAddrSpec.getHostAddress(component);
         ASSERT_STR_EQUAL("example.com", component.data());

         // Attempt to parse "<bigtoken>:user:password@example.com", which
         // cannot be done, because an implicit sip: would cause <bigtoken>
         // to be the user and "user:password" to be the password.  But
         // a password cannot contain ":".

         UtlString bigscheme2;
         bigscheme2.append(bigtoken);
         bigscheme2.append(":user:password@example.com");

         PARSE(bigscheme2);

         CPPUNIT_ASSERT_EQUAL(Url::UnknownUrlScheme, bigscheme2Url.getScheme());

         Url bigSchemeAddrSpec2(bigscheme2, TRUE /* as addr-spec */);

         CPPUNIT_ASSERT_EQUAL(Url::UnknownUrlScheme, bigscheme2Url.getScheme());
      }

   void testBigUriUser()
      {
         // sip:<bigtoken>@example.com

         UtlString biguser;
         biguser.append("sip:");
         biguser.append(bigtoken);
         biguser.append("@example.com");

         PARSE(biguser);

         CPPUNIT_ASSERT_EQUAL(Url::SipUrlScheme, biguserUrl.getScheme());

         UtlString component;

         biguserUrl.getUserId(component);
         CPPUNIT_ASSERT(component.compareTo(bigtoken) == 0);

         biguserUrl.getHostAddress(component);
         CPPUNIT_ASSERT(! component.compareTo("example.com"));
      }

   void testBigUriNoSchemeUser()
      {
         // <bigtoken>@example.com

         UtlString bigusernoscheme;

         bigusernoscheme.append(bigtoken);
         bigusernoscheme.append("@example.com");

         PARSE(bigusernoscheme);

         CPPUNIT_ASSERT_EQUAL(Url::SipUrlScheme, bigusernoschemeUrl.getScheme());

         UtlString component;

         bigusernoschemeUrl.getUserId(component);
         CPPUNIT_ASSERT(component.compareTo(bigtoken) == 0);

         bigusernoschemeUrl.getHostAddress(component);
         CPPUNIT_ASSERT(! component.compareTo("example.com"));
      }

   void testBigUriHost()
      {

         // see if a 128 character host parses ok
         UtlString okhost("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
         UtlString bigok;
         bigok.append("sip:user@");
         bigok.append(okhost);

         PARSE(bigok);

         CPPUNIT_ASSERT_EQUAL(Url::SipUrlScheme, bigokUrl.getScheme());

         UtlString component;

         bigokUrl.getUserId(component);
         CPPUNIT_ASSERT(!component.compareTo("user"));

         bigokUrl.getHostAddress(component);
         CPPUNIT_ASSERT(!component.compareTo(okhost));

         // user@<bigtoken>
         /*
          * The Url class can now handle very large host names in a reasonable
          * time.  Thus, this string parses correctly now.
          */

         UtlString bighost;

         bighost.append("sip:user@");
         bighost.append(bigtoken);

         PARSE(bighost);

         CPPUNIT_ASSERT_EQUAL(Url::SipUrlScheme, bighostUrl.getScheme());
      }

   void testGRUU()
      {
         UtlString url;

         url = "sip:user@example.edu";
         Url url1(url, Url::AddrSpec);
         CPPUNIT_ASSERT(! url1.isGRUU());

         url = "<sip:user@example.edu>;gr=sk33tyzzx";
         Url url2(url, Url::NameAddr);
         CPPUNIT_ASSERT(! url2.isGRUU());

         url = "sip:user@example.edu;gr=sk33tyzzx";
         Url url3(url, Url::AddrSpec);
         CPPUNIT_ASSERT(url3.isGRUU());

         url = "sip:user@example.edu;gr=sk33tyzzx";
         Url url4(url, Url::NameAddr);
         CPPUNIT_ASSERT(! url4.isGRUU());

         url = "sip:user@example.edu;Gr=sk33tyzzx";
         Url url5(url, Url::AddrSpec);
         CPPUNIT_ASSERT(url5.isGRUU());

         url = "sip:user@example.edu;GR=sk33tyzzx";
         Url url6(url, Url::AddrSpec);
         CPPUNIT_ASSERT(url6.isGRUU());

         url = "sip:user@example.edu;gr";
         Url url7(url, Url::AddrSpec);
         CPPUNIT_ASSERT(url7.isGRUU());

         // Note that the URI parameter below is 'gruu', not 'gr'.
         url = "sip:user@example.edu;gruu=sk33tyzzx";
         Url url8(url, Url::AddrSpec);
         CPPUNIT_ASSERT(! url8.isGRUU());

         UtlString gruuId("sk33tyzzx");

         url = "sip:user@example.edu";
         Url url9(url, Url::AddrSpec);
         CPPUNIT_ASSERT(! url9.isGRUU());
         url9.setGRUU(gruuId);
         CPPUNIT_ASSERT(url9.isGRUU());
         UtlString url9_addrspec;
         url9.getUri(url9_addrspec);
         ASSERT_STR_EQUAL("sip:user@example.edu;gr=sk33tyzzx", url9_addrspec);
         UtlString url9_nameaddr;
         url9.toString(url9_nameaddr);
         ASSERT_STR_EQUAL("<sip:user@example.edu;gr=sk33tyzzx>", url9_nameaddr);

         url = "sip:user@example.edu;gr=other";
         Url url10(url, Url::AddrSpec);
         CPPUNIT_ASSERT(url10.isGRUU());
         url10.setGRUU(gruuId);
         CPPUNIT_ASSERT(url10.isGRUU());
         UtlString url10_addrspec;
         url10.getUri(url10_addrspec);
         ASSERT_STR_EQUAL("sip:user@example.edu;gr=sk33tyzzx", url10_addrspec);
         UtlString url10_nameaddr;
         url10.toString(url10_nameaddr);
         ASSERT_STR_EQUAL("<sip:user@example.edu;gr=sk33tyzzx>", url10_nameaddr);

         url = "sip:user@example.edu;GR=other";
         Url url11(url, Url::AddrSpec);
         CPPUNIT_ASSERT(url11.isGRUU());
         url11.setGRUU(gruuId);
         CPPUNIT_ASSERT(url11.isGRUU());
         UtlString url11_addrspec;
         url11.getUri(url11_addrspec);
         ASSERT_STR_EQUAL("sip:user@example.edu;GR=sk33tyzzx", url11_addrspec);
         UtlString url11_nameaddr;
         url11.toString(url11_nameaddr);
         ASSERT_STR_EQUAL("<sip:user@example.edu;GR=sk33tyzzx>", url11_nameaddr);

         url = "sip:user@example.edu";
         Url url12(url, Url::AddrSpec);
         CPPUNIT_ASSERT(! url12.isGRUU());
         UtlString noId;
         url12.setGRUU(noId);
         CPPUNIT_ASSERT(url12.isGRUU());
         UtlString url12_addrspec;
         url12.getUri(url12_addrspec);
         ASSERT_STR_EQUAL("sip:user@example.edu;gr", url12_addrspec);
         UtlString url12_nameaddr;
         url12.toString(url12_nameaddr);
         ASSERT_STR_EQUAL("<sip:user@example.edu;gr>", url12_nameaddr);
      }

   void testErrors()
      {
         // The structure that describes a single test.
         struct test {          
            const char* input_string; // input string
            Url::UriForm uri_form;    // parsing mode
            bool expected_return;     // expected return from fromString()
            const char* output_string; // expected value of toString()
                                       // if fromString() returns true
            const char* next_uri;      // expected value of nextUri
                                       // or NULL to indicate no nextUri should be provided
         };

         // The tests.
         struct test tests[] =
            {
               // Field parameter containing comma, which is a troublesome case.
               { "<sip:2048@10.1.1.126>;methods=\"INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\"",
                 Url::NameAddr, true,
                 "<sip:2048@10.1.1.126>;methods=\"INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\"",
                 "" },
               { "<sip:2048@10.1.1.126>;methods=\"INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\",foo-bar",
                 Url::NameAddr, true,
                 "<sip:2048@10.1.1.126>;methods=\"INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\"",
                 "foo-bar" },
               { "<sip:2048@10.1.1.126>;methods=\"INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\",foo-bar",
                 Url::NameAddr, false,
                 NULL,
                 NULL },
               // AddrSpec without nextUri
               { "sip:foo@bar", Url::AddrSpec, true, "sip:foo@bar", NULL },
               { " sip:foo@bar", Url::AddrSpec, false, NULL, NULL },
               { "sip:foo@bar ", Url::AddrSpec, false, NULL, NULL },
               { "sip:foo@bar,", Url::AddrSpec, false, NULL, NULL },
               // AddrSpec with nextUri
               { "sip:foo@bar", Url::AddrSpec, true, "sip:foo@bar", "" },
               { " sip:foo@bar", Url::AddrSpec, false, NULL, "" },
               { "sip:foo@bar ", Url::AddrSpec, false, NULL, "" },
               { "sip:foo@bar,", Url::AddrSpec, true, "sip:foo@bar", "" },
               { "sip:foo@bar, ", Url::AddrSpec, true, "sip:foo@bar", " " },
               { "sip:foo@bar,x", Url::AddrSpec, true, "sip:foo@bar", "x" },
               { "sip:foo@bar,xyz", Url::AddrSpec, true, "sip:foo@bar", "xyz" },
               // AddrSpec addtional error cases
               { "<sip:foo@bar>", Url::AddrSpec, false, NULL, "" },
               { "FOO sip:foo@bar", Url::AddrSpec, false, NULL, "" },
               { "\"FOO\"<sip:foo@bar>", Url::AddrSpec, false, NULL, "" },
               { "a!", Url::AddrSpec, false, NULL, "" },
               // NameAddr without nextUri
               { "sip:foo@bar", Url::NameAddr, true, "sip:foo@bar", NULL },
               { " sip:foo@bar", Url::NameAddr, true, "sip:foo@bar", NULL },
               { "sip:foo@bar ", Url::NameAddr, true, "sip:foo@bar", NULL },
               { "sip:foo@bar,", Url::NameAddr, false, NULL, NULL },
               // NameAddr with nextUri
               { "sip:foo@bar", Url::NameAddr, true, "sip:foo@bar", "" },
               { " sip:foo@bar", Url::NameAddr, true, "sip:foo@bar", "" },
               { "sip:foo@bar ", Url::NameAddr, true, "sip:foo@bar", "" },
               { "sip:foo@bar,", Url::NameAddr, true, "sip:foo@bar", "" },
               { "sip:foo@bar, ", Url::NameAddr, true, "sip:foo@bar", "" },
               { "sip:foo@bar,x", Url::NameAddr, true, "sip:foo@bar", "x" },
               { "sip:foo@bar,xyz", Url::NameAddr, true, "sip:foo@bar", "xyz" },
               { "<sip:foo@bar>", Url::NameAddr, true, "sip:foo@bar", "" },
               { "\"FOO\"<sip:foo@bar>", Url::NameAddr, true, "\"FOO\"<sip:foo@bar>", "" },
               // NameAddr addtional error cases
               { "FOO sip:foo@bar", Url::NameAddr, false, NULL, "" },
               { "a!", Url::NameAddr, false, NULL, "" },
            };


         // Execute the tests.
         for (unsigned int i = 0; i < sizeof (tests) / sizeof (test); i++)
         {
            // The string to describe a test.
            char label[1000];
            sprintf(label,
                    "item %d: Url::fromString('%s', %s, %s)",
                    i,
                    tests[i].input_string,
                    tests[i].uri_form == Url::AddrSpec ? "AddrSpec" : "NameAddr",
                    tests[i].next_uri ? "&nextUri" : "NULL");

            // Verify that if the expected return is true, then a non-NULL
            // fromString() value has been supplied.
            CPPUNIT_ASSERT_MESSAGE(label,
                                   !tests[i].expected_return ||
                                   tests[i].output_string);

            // Perform the parse.
            Url url;
            UtlString next_uri;
            bool r = url.fromString(tests[i].input_string,
                                    tests[i].uri_form,
                                    tests[i].next_uri ? &next_uri : NULL);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(label,
                                         tests[i].expected_return,
                                         r);
            if (r)
            {
               UtlString unparsed;
               url.toString(unparsed);
               ASSERT_STR_EQUAL_MESSAGE(label,
                                        tests[i].output_string,
                                        unparsed.data());
               if (tests[i].next_uri) 
               {
                  ASSERT_STR_EQUAL_MESSAGE(label,
                                           tests[i].next_uri,
                                           next_uri.data());
               }
            }
         }
      }

    /////////////////////////
    // Helper Methods

    const char *getParam(const char *szName, Url &url)
    {
        UtlString name(szName);
        if (!url.getUrlParameter(name, *assertValue))
        {
            assertValue->append(MISSING_PARAM);
        }

        return assertValue->data();
    }

    const char *getHeaderParam(const char *szName, Url &url)
    {
        UtlString name(szName);
        if (!url.getHeaderParameter(name, *assertValue))
        {
            assertValue->append(MISSING_PARAM);
        }

        return assertValue->data();
    }

    const char *getFieldParam(const char *szName, Url &url, int ndx)
    {
        UtlString name(szName);
        if (!url.getFieldParameter(name, *assertValue, ndx))
        {
            assertValue->append(MISSING_PARAM);
        }

        return assertValue->data();
    }

    const char *getFieldParam(const char *szName, Url &url)
    {
        UtlString name(szName);
        if (!url.getFieldParameter(name, *assertValue))
        {
            assertValue->append(MISSING_PARAM);
        }

        return assertValue->data();
    }

    const char *toString(const Url& url)
    {
        assertValue->remove(0);
        url.toString(*assertValue);

        return assertValue->data();
    }

    const char *getHostAddress(const Url& url)
    {
        assertValue->remove(0);
        url.getHostAddress(*assertValue);

        return assertValue->data();
    }

    const char *getUrlType(const Url& url)
    {
        assertValue->remove(0);
        url.getUrlType(*assertValue);

        return assertValue->data();
    }

    /** 'url' not declared as const, in order to override the Url:: method **/
    const char *getUri(Url& url)
    {
        assertValue->remove(0);
        url.getUri(*assertValue);

        return assertValue->data();
    }

    /** 'url' not declared as const, in order to override the Url:: method **/
    const char *getPath(Url& url, UtlBoolean withQuery = FALSE)
    {
        assertValue->remove(0);
        url.getPath(*assertValue, withQuery);

        return assertValue->data();
    }

    /** 'url' not declared as const, in order to override the Url:: method **/
    const char *getUserId(Url& url)
    {
        assertValue->remove(0);
        url.getUserId(*assertValue);

        return assertValue->data();
    }

    /** 'url' not declared as const, in order to override the Url:: method **/
    const char *getIdentity(Url& url)
    {
        assertValue->remove(0);
        url.getIdentity(*assertValue);

        return assertValue->data();
    }

    const char *getDisplayName(const Url &url)
    {
        assertValue->remove(0);
        url.getDisplayName(*assertValue);

        return assertValue->data();
    }

    void assertArrayMessage(const char *expectedTokens, UtlString *actual,
        CppUnit::SourceLine sourceLine, std::string msg)
    {
        UtlString expected;
        UtlTokenizer toks(expectedTokens);
        for (int i = 0; toks.next(expected, " "); i++)
        {
            TestUtilities::assertEquals(expected.data(), actual[i].data(),
                sourceLine, msg);
            expected.remove(0);
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(UrlTest);
