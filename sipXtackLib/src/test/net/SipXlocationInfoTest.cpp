/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
#include <string>

#include "utl/UtlSList.h"
#include "net/SipXlocationInfo.h"
#include "os/OsTask.h"

// CONSTANTS
static const char* gSharedSecred = "0EZ2ffpBb3CFYjpjhEnjWK08";

// INVITE with no location info header
const char* gNoHeaderMessage =
    "INVITE sip:user@somewhere.example.com SIP/2.0\r\n"
    "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
    "To: sip:user@somewhere.example.com\r\n"
    /* Remember that whitespace is allowed around this ';', as 'SEMI' is used
    * in RFC 3261, section 25.1, production 'from-spec', etc. */
    "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
    "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
    "Cseq: 1 INVITE\r\n"
    "Max-Forwards: 20\r\n"
    "Contact: caller@127.0.0.1\r\n"
    "Content-Length: 0\r\n"
    "\r\n";

typedef struct ParamData
{
  const char* name;
  const char* value;
} ParamData;

// helper struct for tests
typedef struct HeaderData
{
  const char* headerName;
  const char* headerValue;
  const char* identity;
  ParamData param1;
  ParamData param2;
  const char* encodedUri;
} HeaderData;

// more invalid location info headers
HeaderData invalidHeaderData[] =
{
    //X-Sipx-Location-Info: "<" <identity>;location=<location>;signature=<signature-hash> ">"
    {"X-Sipx-Location-Info", "<sip:user@example;location=;signature=bla>", "", {"", ""}, {"", ""}, ""}, //identity,  empty location, invalid signature
    {" ", " ", "",  {"", ""}, {"", ""}, ""},//empty
    {"X-Sipx-Location-Info", "Blah-Blah", "",  {"", ""}, {"", ""}, ""}, //invalid content
    {"X-Sipx-Location-Info", "<sip:user@example>", "", {"", ""}, {"", ""}, ""}, //identity only, no location, no signature
    {"X-Sipx-Location-Info", "<sip:user@example;location=loc;>",  "", {"", ""}, {"", ""}, ""}, //identity,  location, no signature
    {"X-Sipx-Location-Info", "<sip:user@example;location=loc;signature=bla>",  "", {"", ""}, {"", ""}, ""}, //identity,  location, invalid signature
    {"", "", "",  {"", ""}, {"", ""}, ""},
};

// more valid location info headers
HeaderData validHeaderOneParamData[] =
{
    //X-Sipx-Location-Info: "<" <identity>;location=<location>;signature=<signature-hash> ">"
    {"X-Sipx-Location-Info", "<sip:201@dtezuce.ro;x-sipX-location=some_branch_1;signature=9acfb58b3d0a7a40ad33b1c7e2d58d0c>", "201@dtezuce.ro", {"x-sipX-location", "some_branch_1"}, {"", ""},
        "<sip:?X-SipX-Location-Info=%3Csip%3A201%40dtezuce.ro%3Bx-sipX-location%3Dsome_branch_1%3Bsignature%3D9acfb58b3d0a7a40ad33b1c7e2d58d0c%3E>"},

    {"", "", "",  {"", ""}, {"", ""}, ""},
};

HeaderData validHeaderTwoParamsData[] =
{
    //X-Sipx-Location-Info: "<" <identity>;location=<location>;signature=<signature-hash> ">"
    {"X-Sipx-Location-Info", "<sip:201@dtezuce.ro;sipxecs-lineid=1;x-sipX-location=some_branch_1;signature=7ca4441ff66f6dab4f88f2ef4f44f02f>", "201@dtezuce.ro", {"x-sipX-location", "some_branch_1"}, {"sipxecs-lineid", "1"}, "<sip:?X-SipX-Location-Info=%3Csip%3A201%40dtezuce.ro%3Blocation%3Dsome_branch_1%3Bsignature%3De3301705ecbb8995791379ee8d0db184%3E>"},
    {"", "", "",  {"", ""}, {"", ""}, ""},
};

/**
 * Unit test for SipXSignedHeader
 *
 */
class SipXSignedHeaderTest : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(SipXSignedHeaderTest);

//  CPPUNIT_TEST(SipXSignedHeader_DefaultConstructorTest);
//  CPPUNIT_TEST(SipXSignedHeader_ConstructorNoLIheaderTest);
//  CPPUNIT_TEST(SipXSignedHeader_ConstructorInvalidLIheaderTest);
//  CPPUNIT_TEST(SipXSignedHeader_ConstructorValidLIheaderTest);
//  CPPUNIT_TEST(SipXSignedHeader_ConstructorMoreLIheadersTest);

//  CPPUNIT_TEST(SipXSignedHeader_setParamTest);
  CPPUNIT_TEST(SipXSignedHeader_encodeTest);
  CPPUNIT_TEST(SipXSignedHeader_decodeTest);
  CPPUNIT_TEST(SipXSignedHeader_decodeHeaderTest);
  CPPUNIT_TEST(SipXSignedHeader_removeTest);

  CPPUNIT_TEST_SUITE_END();

private:

public:

  void setUp()
  {
    SipXSignedHeader::setSecret(gSharedSecred);
  }

  void tearDown()
  {
  }

  void checkEmptyHeaderInfo(SipXSignedHeader& header, const UtlString&  identity, const UtlString& headerName)
  {
    //TEST: check that everything is empty
    CPPUNIT_ASSERT(0 == identity.compareTo(header._identity));
    CPPUNIT_ASSERT(0 == headerName.compareTo(header._headerName));
    CPPUNIT_ASSERT(header._sSignatureSecret == gSharedSecred);
    //TEST: check that is valid is false

    CPPUNIT_ASSERT(true == header._isValid);

    UtlString actualIdentity = "bad";
    header._encodedUrl.getIdentity(actualIdentity);
    CPPUNIT_ASSERT(0 == actualIdentity.compareTo(header._identity));
    CPPUNIT_ASSERT(Url::SipUrlScheme == header._encodedUrl.getScheme());


    actualIdentity.remove(0);
    //TEST: getters should retrieve OK
    CPPUNIT_ASSERT(header.getIdentity(actualIdentity));
    CPPUNIT_ASSERT(0 == actualIdentity.compareTo(identity));

    UtlString paramValue;
    CPPUNIT_ASSERT(!header.getParam("some_param", paramValue));
    CPPUNIT_ASSERT(paramValue.isNull());

    Url uri;
    //TEST: encoding should work
    CPPUNIT_ASSERT(header.encodeUri(uri));
  }

  void checkNoHeaderInfo(SipXSignedHeader& header, const UtlString& headerName = "")
  {
    //TEST: check that everything is empty
    CPPUNIT_ASSERT(header._identity.isNull());
    CPPUNIT_ASSERT(0 == headerName.compareTo(header._headerName));
    CPPUNIT_ASSERT(header._sSignatureSecret == gSharedSecred);
    //TEST: check that is valid is false

    CPPUNIT_ASSERT(false == header._isValid);

    UtlString actualIdentity;
    //TEST: getters should not retrieve anything
    CPPUNIT_ASSERT(!header.getIdentity(actualIdentity));

    UtlString paramValue;
    CPPUNIT_ASSERT(!header.getParam("some_param", paramValue));
    CPPUNIT_ASSERT(paramValue.isNull());

    Url uri;
    //TEST: encoding should fail
    CPPUNIT_ASSERT(!header.encodeUri(uri));
  }

  void checkHeaderInfo(SipXSignedHeader& header, const HeaderData& expected, bool check2ndParam = false )
  {
    UtlString actualIdentity = "badcontent";
    UtlString actualLocation = "badcontent";

    //TEST: check that everything is set
    CPPUNIT_ASSERT(!header._identity.isNull());
    CPPUNIT_ASSERT(header._sSignatureSecret == gSharedSecred);
    //TEST: check that is valid is true
    CPPUNIT_ASSERT(header._isValid);

    //TEST: getters should retrieve expected
    CPPUNIT_ASSERT(header.getIdentity(actualIdentity));
    CPPUNIT_ASSERT(actualIdentity == expected.identity);
    UtlString actualParamValue;
    CPPUNIT_ASSERT(header.getParam(expected.param1.name, actualParamValue));
    CPPUNIT_ASSERT(0 == actualParamValue.compareTo(expected.param1.value));
    if (check2ndParam)
    {
      CPPUNIT_ASSERT(header.getParam(expected.param2.name, actualParamValue));
      CPPUNIT_ASSERT(0 == actualParamValue.compareTo(expected.param2.value));
    }

    Url uri;
    //TEST: encoding should work
    CPPUNIT_ASSERT(header.encodeUri(uri));
    CPPUNIT_ASSERT(uri.toString() == expected.encodedUri);
  }

  void SipXSignedHeader_DefaultConstructorTest()
  {
    // construct an object using the default constructor
    const UtlString identity = "201@dtezuce.ro";
    const UtlString headerName = "X-SipX-Some-Header";
    SipXSignedHeader header(identity, headerName);

    checkEmptyHeaderInfo(header, identity, headerName);
  }

  void SipXSignedHeader_ConstructorNoLIheaderTest()
  {
    // construct an object using the message param constructor but with a message with no header
    SipMessage message(gNoHeaderMessage, strlen(gNoHeaderMessage));
    SipXSignedHeader header(message, "noheader");
    checkNoHeaderInfo(header, "noheader");
  }

  void SipXSignedHeader_ConstructorInvalidLIheaderTest()
  {
    SipMessage message(gNoHeaderMessage, strlen(gNoHeaderMessage));

    for(int i = 0; ; i++)
    {
      if ('\0' == invalidHeaderData[i].headerName[0])
      {
        // no more invalid headers, terminate
        break;
      }

      // remove previous header and add next invalid header
      message.removeHeader(invalidHeaderData[i].headerName, 0);
      message.addHeaderField(invalidHeaderData[i].headerName, invalidHeaderData[i].headerValue);

      SipXSignedHeader header(message, invalidHeaderData[i].headerName);
      checkNoHeaderInfo(header, invalidHeaderData[i].headerName);
    }
  }

  void SipXSignedHeader_ConstructorValidLIheaderTest()
  {
    SipMessage message(gNoHeaderMessage, strlen(gNoHeaderMessage));

    for(int i = 0; ; i++)
    {
      if ('\0' == validHeaderOneParamData[i].headerName[0])
      {
        // no more invalid headers, terminate
        break;
      }

      // remove previous header and add next valid header
      message.removeHeader(validHeaderOneParamData[i].headerName, 0);
      message.addHeaderField(validHeaderOneParamData[i].headerName, validHeaderOneParamData[i].headerValue);

      SipXSignedHeader header(message, validHeaderOneParamData[i].headerName);
      checkHeaderInfo(header, validHeaderOneParamData[i]);
    }

//    for(int i = 0; ; i++)
//    {
//      if ('\0' == validHeaderTwoParamsData[i].headerName[0])
//      {
//        // no more invalid headers, terminate
//        break;
//      }
//
//      // remove previous header and add next valid header
//      message.removeHeader(validHeaderTwoParamsData[i].headerName, 0);
//      message.addHeaderField(validHeaderTwoParamsData[i].headerName, validHeaderTwoParamsData[i].headerValue);
//
//      SipXSignedHeader header(message, validHeaderTwoParamsData[i].headerName);
//      checkHeaderInfo(header, validHeaderTwoParamsData[i]);
//    }

  }

  void SipXSignedHeader_ConstructorMoreLIheadersTest()
  {
    // TEST: more than one header, first valid, last invalid
    {
      SipMessage message(gNoHeaderMessage, strlen(gNoHeaderMessage));

      message.addHeaderField(validHeaderOneParamData[0].headerName, validHeaderOneParamData[0].headerValue);
      message.addHeaderField(invalidHeaderData[0].headerName, invalidHeaderData[0].headerValue);

      SipXSignedHeader header(message, invalidHeaderData[0].headerName);
      checkNoHeaderInfo(header, invalidHeaderData[0].headerName);
    }

    // TEST: more than one header, first invalid, last valid
    {
      SipMessage message(gNoHeaderMessage, strlen(gNoHeaderMessage));

      // remove previous header and add next invalid header
      message.addHeaderField(invalidHeaderData[0].headerName, invalidHeaderData[0].headerValue);
      message.addHeaderField(validHeaderOneParamData[0].headerName, validHeaderOneParamData[0].headerValue);

      SipXSignedHeader header(message, validHeaderOneParamData[0].headerName);
      checkHeaderInfo(header, validHeaderOneParamData[0]);
    }

    // TEST: more than one header, all valid
    {
      SipMessage message(gNoHeaderMessage, strlen(gNoHeaderMessage));

      message.addHeaderField(validHeaderOneParamData[0].headerName, validHeaderOneParamData[0].headerValue);
      message.addHeaderField(validHeaderTwoParamsData[0].headerName, validHeaderTwoParamsData[1].headerValue);

      SipXSignedHeader header(message, validHeaderTwoParamsData[0].headerName);
      checkHeaderInfo(header, validHeaderTwoParamsData[1], true);
    }
  }

  void SipXSignedHeader_setParamTest()
  {
    for(int i = 0; ; i++)
    {
      if ('\0' == validHeaderOneParamData[i].headerName[0])
      {
        // no more invalid headers, terminate
        break;
      }

      UtlString identity = validHeaderOneParamData[i].identity;
      SipXSignedHeader header(identity, validHeaderTwoParamsData[i].headerName);
      //TEST: setInfo should work
      header.setParam(validHeaderOneParamData[i].param1.name, validHeaderOneParamData[i].param1.value);
      //TEST: there should be location info
      checkHeaderInfo(header, validHeaderOneParamData[i]);
    }

//    for(int i = 0; ; i++)
//    {
//      if ('\0' == validHeaderTwoParamsData[i].headerName[0])
//      {
//        // no more invalid headers, terminate
//        break;
//      }
//
//      UtlString identity = validHeaderTwoParamsData[i].identity;
//      SipXSignedHeader header(identity, validHeaderTwoParamsData[i].headerName);
//      //TEST: setInfo should work
//      header.setParam(validHeaderTwoParamsData[i].param1.name, validHeaderTwoParamsData[i].param1.value);
//      header.setParam(validHeaderTwoParamsData[i].param2.name, validHeaderTwoParamsData[i].param2.value);
//      //TEST: there should be location info
//      checkHeaderInfo(header, validHeaderTwoParamsData[i], true);
//    }
  }

  void SipXSignedHeader_encodeTest()
  {
    for(int i = 0; ; i++)
    {
      if ('\0' == validHeaderOneParamData[i].headerName[0])
      {
        // no more invalid headers, terminate
        break;
      }

      UtlString identity = validHeaderOneParamData[i].identity;
      SipXSignedHeader header(identity, validHeaderTwoParamsData[i].headerName);
      // set something
      header.setParam(validHeaderOneParamData[i].param1.name, validHeaderOneParamData[i].param1.value);

      UtlString actualHeader;
      //TEST: encode should work
      header.encode(actualHeader);
      //TEST: encode should give expected header
      CPPUNIT_ASSERT(actualHeader == validHeaderOneParamData[i].headerValue);
    }

    for(int i = 0; ; i++)
    {
      if ('\0' == validHeaderTwoParamsData[i].headerName[0])
      {
        // no more invalid headers, terminate
        break;
      }

      UtlString identity = validHeaderOneParamData[i].identity;
      SipXSignedHeader header(identity, validHeaderTwoParamsData[i].headerName);
      // set something
      header.setParam(validHeaderTwoParamsData[i].param1.name, validHeaderTwoParamsData[i].param1.value);
      header.setParam(validHeaderTwoParamsData[i].param2.name, validHeaderTwoParamsData[i].param2.value);

      UtlString actualHeader;
      //TEST: encode should work
      header.encode(actualHeader);
      //TEST: encode should give expected header
      CPPUNIT_ASSERT(actualHeader == validHeaderTwoParamsData[i].headerValue);
    }
  }

  void SipXSignedHeader_decodeTest()
  {
    // NOTE: This is implemented by the constructor tests as the constructor is a wrapper over decode()
  }

  void SipXSignedHeader_decodeHeaderTest()
  {
    for(int i = 0; ; i++)
    {
      if ('\0' == invalidHeaderData[i].headerName[0])
      {
        // no more invalid headers, terminate
        break;
      }

      UtlString identity = validHeaderOneParamData[i].identity;
      SipXSignedHeader header(identity, invalidHeaderData[i].headerName);
      //TEST: decode should fail for invalid headers
      CPPUNIT_ASSERT(!header.decodeHeader(invalidHeaderData[i].headerValue));
      //TEST: there should be no location info after a failed decode
      checkNoHeaderInfo(header, invalidHeaderData[i].headerName);
    }

    SipMessage message(gNoHeaderMessage, strlen(gNoHeaderMessage));

    for(int i = 0; ; i++)
    {
      if ('\0' == validHeaderOneParamData[i].headerName[0])
      {
        // no more invalid headers, terminate
        break;
      }

      UtlString identity = validHeaderOneParamData[i].identity;
      SipXSignedHeader header(identity, validHeaderOneParamData[i].headerName);
      //TEST: decode should work for valid headers
      CPPUNIT_ASSERT(header.decodeHeader(validHeaderOneParamData[i].headerValue));
      //TEST: there should be location info after a good decode
      checkHeaderInfo(header, validHeaderOneParamData[i]);
    }

    for(int i = 0; ; i++)
    {
      if ('\0' == validHeaderTwoParamsData[i].headerName[0])
      {
        // no more invalid headers, terminate
        break;
      }

      UtlString identity = validHeaderOneParamData[i].identity;
      SipXSignedHeader header(identity, validHeaderTwoParamsData[i].headerName);
      //TEST: decode should work for valid headers
      CPPUNIT_ASSERT(header.decodeHeader(validHeaderTwoParamsData[i].headerValue));
      //TEST: there should be location info after a good decode
      checkHeaderInfo(header, validHeaderTwoParamsData[i], true);
    }
  }

   void SipXSignedHeader_removeTest()
   {

     // TEST: remove should work over an INVITE with no location info header
     {
       SipMessage message(gNoHeaderMessage, strlen(gNoHeaderMessage));
       //call remove
       SipXSignedHeader::remove(message, invalidHeaderData[0].headerName);

       SipXSignedHeader header(message, invalidHeaderData[0].headerName);
       // TEST: there should be no location info in the message after remove
       checkNoHeaderInfo(header, invalidHeaderData[0].headerName);
     }

     // TEST: remove should remove from an INVITE a location info header
     {
       SipMessage message(gNoHeaderMessage, strlen(gNoHeaderMessage));
       UtlString headerName = validHeaderOneParamData[0].headerName;
       // add a location info header in the message
       message.addHeaderField(headerName, validHeaderOneParamData[0].headerValue);
       SipXSignedHeader header(message, headerName);
       // check the header was added correctly
       checkHeaderInfo(header, validHeaderOneParamData[0]);

       SipXSignedHeader::remove(message, headerName);

       SipXSignedHeader noLocationInfo(message, headerName);
       // TEST: there should be no location info in the message after remove
       checkNoHeaderInfo(noLocationInfo, headerName);
     }
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipXSignedHeaderTest);
