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
const char* gNoLIheaderMessage =
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

// helper struct for tests
typedef struct LIheaderData
{
  const char* LIheader;
  const char* identity;
  const char* location;
  const char* encodedUri;
} LIheaderData;

// more invalid location info headers
LIheaderData invalidLIheaderData[] =
{
    //X-Sipx-Location-Info: "<" <identity>;location=<location>;signature=<signature-hash> ">"
    {"<sip:user@example;location=;signature=bla>", "", "", ""}, //identity,  empty location, invalid signature
    {" ", "", "", ""},//empty
    {"Blah-Blah", "", "", ""}, //invalid content
    {"<sip:user@example>", "", "", ""}, //identity only, no location, no signature
    {"<sip:user@example;location=loc;>", "", "", ""}, //identity,  location, no signature
    {"<sip:user@example;location=loc;signature=bla>", "", "", ""}, //identity,  location, invalid signature
    {"","","",""},
};

// more valid location info headers
LIheaderData validLIheaderData[] =
{
    //X-Sipx-Location-Info: "<" <identity>;location=<location>;signature=<signature-hash> ">"
    {"<sip:201@dtezuce.ro;location=some_branch_1;signature=e3301705ecbb8995791379ee8d0db184>", "201@dtezuce.ro", "some_branch_1", "<sip:?X-SipX-Location-Info=%3Csip%3A201%40dtezuce.ro%3Blocation%3Dsome_branch_1%3Bsignature%3De3301705ecbb8995791379ee8d0db184%3E>"},
    {"","","",""},
};


/**
 * Unit test for SipXlocationInfo
 *
 */
class SipXlocationInfoTest : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(SipXlocationInfoTest);

  CPPUNIT_TEST(SipXlocationInfo_DefaultConstructorTest);
  CPPUNIT_TEST(SipXlocationInfo_ConstructorNoLIheaderTest);
  CPPUNIT_TEST(SipXlocationInfo_ConstructorInvalidLIheaderTest);
  CPPUNIT_TEST(SipXlocationInfo_ConstructorValidLIheaderTest);
  CPPUNIT_TEST(SipXlocationInfo_ConstructorMoreLIheadersTest);

  CPPUNIT_TEST(SipXlocationInfo_setInfoTest);
  CPPUNIT_TEST(SipXlocationInfo_encodeTest);
  CPPUNIT_TEST(SipXlocationInfo_decodeTest);
  CPPUNIT_TEST(SipXlocationInfo_decodeHeaderTest);
  CPPUNIT_TEST(SipXlocationInfo_removeTest);

  CPPUNIT_TEST_SUITE_END();

private:

public:

  void setUp()
  {
    SipXlocationInfo::setSecret(gSharedSecred);
  }

  void tearDown()
  {
  }

  void checkNoLocationInfo(SipXlocationInfo& locationInfo)
  {
    UtlString identity = "badcontent";
    UtlString location = "badcontent";

    //TEST: check that everything is empty
    CPPUNIT_ASSERT(locationInfo._identity.isNull());
    CPPUNIT_ASSERT(locationInfo._location.isNull());
    CPPUNIT_ASSERT(locationInfo._sSignatureSecret == gSharedSecred);
    //TEST: check that is valid is false
    CPPUNIT_ASSERT(false == locationInfo._isValid);

    //TEST: getters should not retrieve anything
    CPPUNIT_ASSERT(!locationInfo.getIdentity(identity));
    CPPUNIT_ASSERT(identity.isNull());
    CPPUNIT_ASSERT(!locationInfo.getLocation(location));
    CPPUNIT_ASSERT(location.isNull());

    Url uri;
    //TEST: encoding should fail
    CPPUNIT_ASSERT(!locationInfo.encodeUri(uri));
  }

  void checkLocationInfo(SipXlocationInfo& locationInfo, const LIheaderData& expected)
  {
    UtlString actualIdentity = "badcontent";
    UtlString actualLocation = "badcontent";

    //TEST: check that everything is set
    CPPUNIT_ASSERT(!locationInfo._identity.isNull());
    CPPUNIT_ASSERT(!locationInfo._location.isNull());
    CPPUNIT_ASSERT(locationInfo._sSignatureSecret == gSharedSecred);
    //TEST: check that is valid is true
    CPPUNIT_ASSERT(locationInfo._isValid);

    //TEST: getters should retrieve expected
    CPPUNIT_ASSERT(locationInfo.getIdentity(actualIdentity));
    CPPUNIT_ASSERT(actualIdentity == expected.identity);
    CPPUNIT_ASSERT(locationInfo.getLocation(actualLocation));
    CPPUNIT_ASSERT(actualLocation == expected.location);

    Url uri;
    //TEST: encoding should work
    CPPUNIT_ASSERT(locationInfo.encodeUri(uri));
    CPPUNIT_ASSERT(uri.toString() == expected.encodedUri);
  }

  void SipXlocationInfo_DefaultConstructorTest()
  {
    // construct an object using the default constructor
    SipXlocationInfo locationInfo;
    checkNoLocationInfo(locationInfo);
  }

  void SipXlocationInfo_ConstructorNoLIheaderTest()
  {
    // construct an object using the message param constructor but with a message with no header
    SipMessage message(gNoLIheaderMessage, strlen(gNoLIheaderMessage));
    SipXlocationInfo locationInfo(message);
    checkNoLocationInfo(locationInfo);
  }

  void SipXlocationInfo_ConstructorInvalidLIheaderTest()
  {
    SipMessage message(gNoLIheaderMessage, strlen(gNoLIheaderMessage));

    for(int i = 0; ; i++)
    {
      if ('\0' == invalidLIheaderData[i].LIheader[0])
      {
        // no more invalid headers, terminate
        break;
      }

      // remove previous header and add next invalid header
      message.removeHeader(SipXlocationInfo::HeaderName, 0);
      message.addHeaderField(SipXlocationInfo::HeaderName, invalidLIheaderData[i].LIheader);

      SipXlocationInfo locationInfo(message);
      checkNoLocationInfo(locationInfo);
    }
  }

  void SipXlocationInfo_ConstructorValidLIheaderTest()
  {
    SipMessage message(gNoLIheaderMessage, strlen(gNoLIheaderMessage));

    for(int i = 0; ; i++)
    {
      if ('\0' == validLIheaderData[i].LIheader[0])
      {
        // no more invalid headers, terminate
        break;
      }

      // remove previous header and add next valid header
      message.removeHeader(SipXlocationInfo::HeaderName, 0);
      message.addHeaderField(SipXlocationInfo::HeaderName, validLIheaderData[i].LIheader);

      SipXlocationInfo locationInfo(message);
      checkLocationInfo(locationInfo, validLIheaderData[i]);
    }
  }

  void SipXlocationInfo_ConstructorMoreLIheadersTest()
  {
    // TEST: more than one header, first valid, last invalid
    {
      SipMessage message(gNoLIheaderMessage, strlen(gNoLIheaderMessage));

      message.addHeaderField(SipXlocationInfo::HeaderName, validLIheaderData[0].LIheader);
      message.addHeaderField(SipXlocationInfo::HeaderName, invalidLIheaderData[0].LIheader);

      SipXlocationInfo locationInfo(message);
      checkNoLocationInfo(locationInfo);
    }

    // TEST: more than one header, first invalid, last valid
    {
      SipMessage message(gNoLIheaderMessage, strlen(gNoLIheaderMessage));

      // remove previous header and add next invalid header
      message.addHeaderField(SipXlocationInfo::HeaderName, invalidLIheaderData[0].LIheader);
      message.addHeaderField(SipXlocationInfo::HeaderName, validLIheaderData[0].LIheader);


      SipXlocationInfo locationInfo(message);
      checkLocationInfo(locationInfo, validLIheaderData[0]);
    }

    // TEST: more than one header, all valid
    {
      SipMessage message(gNoLIheaderMessage, strlen(gNoLIheaderMessage));

      message.addHeaderField(SipXlocationInfo::HeaderName, validLIheaderData[0].LIheader);
      message.addHeaderField(SipXlocationInfo::HeaderName, validLIheaderData[0].LIheader);

      SipXlocationInfo locationInfo(message);
      checkLocationInfo(locationInfo, validLIheaderData[0]);
    }
  }

  void SipXlocationInfo_setInfoTest()
  {
    for(int i = 0; ; i++)
    {
      if ('\0' == validLIheaderData[i].LIheader[0])
      {
        // no more invalid headers, terminate
        break;
      }

      SipXlocationInfo locationInfo;
      //TEST: setInfo should work
      locationInfo.setInfo(validLIheaderData[i].identity, validLIheaderData[i].location);
      //TEST: there should be location info
      checkLocationInfo(locationInfo, validLIheaderData[i]);
    }
  }

  void SipXlocationInfo_encodeTest()
  {
    for(int i = 0; ; i++)
    {
      if ('\0' == validLIheaderData[i].LIheader[0])
      {
        // no more invalid headers, terminate
        break;
      }

      SipXlocationInfo locationInfo;
      // set something
      locationInfo.setInfo(validLIheaderData[i].identity, validLIheaderData[i].location);

      UtlString actualHeader;
      //TEST: encode should work
      locationInfo.encode(actualHeader);
      //TEST: encode should give expected header
      CPPUNIT_ASSERT(actualHeader == validLIheaderData[i].LIheader);
    }
  }

  void SipXlocationInfo_decodeTest()
  {
    // NOTE: This is implemented by the constructor tests as the constructor is a wrapper over decode()
  }

  void SipXlocationInfo_decodeHeaderTest()
  {
    for(int i = 0; ; i++)
    {
      if ('\0' == invalidLIheaderData[i].LIheader[0])
      {
        // no more invalid headers, terminate
        break;
      }

      SipXlocationInfo locationInfo;
      //TEST: decode should fail for invalid headers
      CPPUNIT_ASSERT(!locationInfo.decodeHeader(invalidLIheaderData[i].LIheader));
      //TEST: there should be no location info after a failed decode
      checkNoLocationInfo(locationInfo);
    }

    SipMessage message(gNoLIheaderMessage, strlen(gNoLIheaderMessage));

    for(int i = 0; ; i++)
    {
      if ('\0' == validLIheaderData[i].LIheader[0])
      {
        // no more invalid headers, terminate
        break;
      }

      SipXlocationInfo locationInfo;
      //TEST: decode should work for valid headers
      CPPUNIT_ASSERT(locationInfo.decodeHeader(validLIheaderData[i].LIheader));
      //TEST: there should be location info after a good decode
      checkLocationInfo(locationInfo, validLIheaderData[i]);
    }
  }

   void SipXlocationInfo_removeTest()
   {

     // TEST: remove should work over an INVITE with no location info header
     {
       SipMessage message(gNoLIheaderMessage, strlen(gNoLIheaderMessage));
       //call remove
       SipXlocationInfo::remove(message);

       SipXlocationInfo locationInfo(message);
       // TEST: there should be no location info in the message after remove
       checkNoLocationInfo(locationInfo);
     }

     // TEST: remove should remove from an INVITE a location info header
     {
       SipMessage message(gNoLIheaderMessage, strlen(gNoLIheaderMessage));
       // add a location info header in the message
       message.addHeaderField(SipXlocationInfo::HeaderName, validLIheaderData[0].LIheader);
       SipXlocationInfo locationInfo(message);
       // check the header was added correctly
       checkLocationInfo(locationInfo, validLIheaderData[0]);

       SipXlocationInfo::remove(message);

       SipXlocationInfo noLocationInfo(message);
       // TEST: there should be no location info in the message after remove
       checkNoLocationInfo(noLocationInfo);
     }
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipXlocationInfoTest);

