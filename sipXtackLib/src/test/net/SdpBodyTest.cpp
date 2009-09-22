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

#include <os/OsSysLog.h>
#include <os/OsDefs.h>
#include <net/HttpMessage.h>
#include <net/SdpBody.h>
#include <net/SdpCodec.h>
#include <net/SdpCodecFactory.h>


/**
 * Unit test for SdpBody
 */
class SdpBodyTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SdpBodyTest);
   CPPUNIT_TEST(testParser);
   CPPUNIT_TEST(testIndexAccessor);
   CPPUNIT_TEST(testBlankLineSkipping);
   CPPUNIT_TEST(testNewMessage);
   CPPUNIT_TEST(testTimeHeaders);
   CPPUNIT_TEST(testGetMediaSetCount);
   CPPUNIT_TEST(testGetMediaAddress);
   CPPUNIT_TEST(testGetMediaAttribute);
   CPPUNIT_TEST(testRemoveMediaAttribute);
   CPPUNIT_TEST(testInsertAttribute);
   CPPUNIT_TEST(testModifyMediaAddress);
   CPPUNIT_TEST(testModifyMediaPort);
   CPPUNIT_TEST(testCandidateParsing);
   CPPUNIT_TEST(testRtcpPortParsing);
   CPPUNIT_TEST(testAddAttribute);
   CPPUNIT_TEST(testInsertTelephoneEvent);
   CPPUNIT_TEST_SUITE_END();

public:

   void testParser()
      {
         const char *sdp =
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;

         SdpBody body(sdp);

         UtlString bodyString;
         ssize_t bodyLength;

         body.getBytes(&bodyString, &bodyLength);

         ASSERT_STR_EQUAL( bodyString.data(), sdp);

         ssize_t otherLength = body.getLength();
         CPPUNIT_ASSERT( bodyLength == otherLength );

         const char *junk = "IRRELEVENT JUNK THAT IS NOT SDP";

         UtlString embeddedSdp(sdp);
         embeddedSdp.append(junk);

         SdpBody embeddedBody(embeddedSdp.data(), strlen(sdp));

         UtlString extractedString;
         ssize_t extractedStringLength;

         embeddedBody.getBytes(&extractedString, &extractedStringLength);

         ASSERT_STR_EQUAL( bodyString.data(), extractedString.data());
      }

   void testIndexAccessor()
      {
         const char *sdp =
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;

         SdpBody body(sdp);

         /* Why are there not accessors for version or other standard fields !?! */

         int fields = body.SdpBody::getFieldCount();
         CPPUNIT_ASSERT(fields == 13);

         UtlString name;
         UtlString value;

         CPPUNIT_ASSERT(body.getValue(0, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "v");
         ASSERT_STR_EQUAL(value.data(), "0");

         CPPUNIT_ASSERT(body.getValue(1, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "o");
         ASSERT_STR_EQUAL(value.data(), "mhandley 2890844526 2890842807 IN IP4 126.16.64.4");

         CPPUNIT_ASSERT(body.getValue(2, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "s");
         ASSERT_STR_EQUAL(value.data(), "SDP Seminar");

         CPPUNIT_ASSERT(body.getValue(3, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "i");
         ASSERT_STR_EQUAL(value.data(), "A Seminar on the session description protocol");

         CPPUNIT_ASSERT(body.getValue(4, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "u");
         ASSERT_STR_EQUAL(value.data(), "http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps");

         CPPUNIT_ASSERT(body.getValue(5, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "e");
         ASSERT_STR_EQUAL(value.data(), "mjh@isi.edu (Mark Handley)");

         CPPUNIT_ASSERT(body.getValue(6, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "c");
         ASSERT_STR_EQUAL(value.data(), "IN IP4 224.2.17.12/127");

         CPPUNIT_ASSERT(body.getValue(7, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "t");
         ASSERT_STR_EQUAL(value.data(), "2873397496 2873404696");

         CPPUNIT_ASSERT(body.getValue(8, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "a");
         ASSERT_STR_EQUAL(value.data(), "recvonly");

         CPPUNIT_ASSERT(body.getValue(9, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "m");
         ASSERT_STR_EQUAL(value.data(), "audio 49170 RTP/AVP 0");

         CPPUNIT_ASSERT(body.getValue(10, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "m");
         ASSERT_STR_EQUAL(value.data(), "video 51372 RTP/AVP 31");

         CPPUNIT_ASSERT(body.getValue(11, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "m");
         ASSERT_STR_EQUAL(value.data(), "application 32416 udp wb");

         CPPUNIT_ASSERT(body.getValue(12, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "a");
         ASSERT_STR_EQUAL(value.data(), "orient:portrait");

         CPPUNIT_ASSERT(!body.getValue(13, &name, &value));
         CPPUNIT_ASSERT(name.isNull());
         CPPUNIT_ASSERT(value.isNull());

         name = "foo";
         value = "notempty";

         CPPUNIT_ASSERT(!body.getValue(15, &name, &value));
         CPPUNIT_ASSERT(name.isNull());
         CPPUNIT_ASSERT(value.isNull());

         name = "foo";
         value = "notempty";

         CPPUNIT_ASSERT(!body.getValue(-2, &name, &value));
         CPPUNIT_ASSERT(value.isNull());
         CPPUNIT_ASSERT(name.isNull());
      }

   void testBlankLineSkipping()
      {
         const char *sdp =
            "\n"
            "\r"
            "\r\n"
            "v=0\r\n"
            "\n"
            "\r"
            "\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "\n"
            "\r"
            "\r\n"
            "s=SDP Seminar\r\n"
            "\n"
            "\r"
            "\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "\n"
            "\r"
            "\r\n"
            ;

         SdpBody body(sdp);

         int fields = body.SdpBody::getFieldCount();
         CPPUNIT_ASSERT(fields == 4);

         UtlString name;
         UtlString value;

         CPPUNIT_ASSERT(body.getValue(0, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "v");
         ASSERT_STR_EQUAL(value.data(), "0");

         CPPUNIT_ASSERT(body.getValue(1, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "o");
         ASSERT_STR_EQUAL(value.data(), "mhandley 2890844526 2890842807 IN IP4 126.16.64.4");

         CPPUNIT_ASSERT(body.getValue(2, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "s");
         ASSERT_STR_EQUAL(value.data(), "SDP Seminar");

         CPPUNIT_ASSERT(body.getValue(3, &name, &value));
         ASSERT_STR_EQUAL(name.data(), "i");
         ASSERT_STR_EQUAL(value.data(), "A Seminar on the session description protocol");
      }

   void testNewMessage()
      {
         UtlString sdpMsg;
         UtlString sdpExpected;
         ssize_t sdpLength;

         SdpBody newSdp;

         const char *expectedStdVersion =
            "v=0\r\n"
            ;

         const char *defaultOriginator = // provided automatically
            "o=sipXecs 5 5 IN IP4 127.0.0.1\r\n"
            ;

         const char *expectedEmptySessionName = // not optional, so will be there but empty
            "s=\r\n"
            ;
         newSdp.getBytes(&sdpMsg, &sdpLength);

         sdpExpected.remove(0);
         sdpExpected.append(expectedStdVersion);
         sdpExpected.append(defaultOriginator);
         sdpExpected.append(expectedEmptySessionName);

         ASSERT_STR_EQUAL(sdpExpected.data(), sdpMsg.data());

         newSdp.setStandardHeaderFields("my session name",
                                        "me@example.org",
                                        "+1-8005551212",
                                        "10.1.1.1"
                                        );

         newSdp.getBytes(&sdpMsg, &sdpLength);

         const char *expectedStdHead =
            "o=sipXecs 5 5 IN IP4 10.1.1.1\r\n"
            "s=my session name\r\n"
            "e=me@example.org\r\n"
            "p=+1-8005551212\r\n"
            ;

         sdpExpected.remove(0);
         sdpExpected.append(expectedStdVersion);
         sdpExpected.append(expectedStdHead);

         ASSERT_STR_EQUAL(sdpExpected.data(), sdpMsg.data());

         newSdp.setPhoneNumberField("+1-7819385306");

         newSdp.getBytes(&sdpMsg, &sdpLength);

         const char *expectedPhone =
            "o=sipXecs 5 5 IN IP4 10.1.1.1\r\n"
            "s=my session name\r\n"
            "e=me@example.org\r\n"
            "p=+1-7819385306\r\n"
            ;

         sdpExpected.remove(0);
         sdpExpected.append(expectedStdVersion);
         sdpExpected.append(expectedPhone);

         ASSERT_STR_EQUAL(sdpExpected.data(), sdpMsg.data());

         newSdp.setEmailAddressField("you@example.com");

         newSdp.getBytes(&sdpMsg, &sdpLength);

         const char *expectedEmail =
            "o=sipXecs 5 5 IN IP4 10.1.1.1\r\n"
            "s=my session name\r\n"
            "e=you@example.com\r\n"
            "p=+1-7819385306\r\n"
            ;

         sdpExpected.remove(0);
         sdpExpected.append(expectedStdVersion);
         sdpExpected.append(expectedEmail);

         ASSERT_STR_EQUAL(sdpExpected.data(), sdpMsg.data());

         newSdp.setSessionNameField("your session name");

         newSdp.getBytes(&sdpMsg, &sdpLength);

         const char *expectedName =
            "o=sipXecs 5 5 IN IP4 10.1.1.1\r\n"
            "s=your session name\r\n"
            "e=you@example.com\r\n"
            "p=+1-7819385306\r\n"
            ;

         sdpExpected.remove(0);
         sdpExpected.append(expectedStdVersion);
         sdpExpected.append(expectedName);

         ASSERT_STR_EQUAL(sdpExpected.data(), sdpMsg.data());

         newSdp.setOriginator("myStream", 10, 20, "10.2.2.2");

         newSdp.getBytes(&sdpMsg, &sdpLength);

         const char *expectedOrigin =
            "o=myStream 10 20 IN IP4 10.2.2.2\r\n"
            "s=your session name\r\n"
            "e=you@example.com\r\n"
            "p=+1-7819385306\r\n"
            ;

         sdpExpected.remove(0);
         sdpExpected.append(expectedStdVersion);
         sdpExpected.append(expectedOrigin);

         ASSERT_STR_EQUAL(sdpExpected.data(), sdpMsg.data());
      }

   void testTimeHeaders()
      {
         const char *sdp =
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;

         SdpBody body(sdp);

         body.addNtpTime( 10000000, 20000000 );

         UtlString sdpMsg;
         ssize_t sdpLength;

         body.getBytes(&sdpMsg, &sdpLength);

         const char *expectedSdp =
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "t=10000000 20000000\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;

         ASSERT_STR_EQUAL(expectedSdp, sdpMsg.data());

         const char *sdp2 =
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;

         SdpBody body2(sdp2);

         body2.addEpochTime( 0, 1 );

         body2.getBytes(&sdpMsg, &sdpLength);

         const char *expectedSdp2 =
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "t=2208988800 2208988801\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;

         ASSERT_STR_EQUAL(expectedSdp2, sdpMsg.data());
      }

   void testGetMediaSetCount()
      {
         const char *sdp1 =
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;
         SdpBody body1(sdp1);

         CPPUNIT_ASSERT(body1.getMediaSetCount() == 3);

         const char *sdp2 =
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "a=orient:portrait\r\n"
            ;
         SdpBody body2(sdp2);

         CPPUNIT_ASSERT(body2.getMediaSetCount() == 0);

         const char *sdp3 =
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "i=Silly Title\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "c=IN IP4 224.2.17.15/127\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;
         SdpBody body3(sdp3);

         CPPUNIT_ASSERT(body1.getMediaSetCount() == 3);
      }

   void testGetMediaAddress()
      {
         UtlString address;

         const char *sdp1 =
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;
         SdpBody body1(sdp1);

         CPPUNIT_ASSERT(body1.getMediaAddress(0, &address));
         ASSERT_STR_EQUAL( address.data(), "224.2.17.12");
         address.remove(0);

         CPPUNIT_ASSERT(body1.getMediaAddress(1, &address));
         ASSERT_STR_EQUAL( address.data(), "224.2.17.12");
         address.remove(0);

         CPPUNIT_ASSERT(body1.getMediaAddress(2, &address));
         ASSERT_STR_EQUAL( address.data(), "224.2.17.12");

         address = "wrong";

         CPPUNIT_ASSERT(!body1.getMediaAddress(3, &address));
         CPPUNIT_ASSERT(address.isNull());

         address = "wrong";

         CPPUNIT_ASSERT(!body1.getMediaAddress(-1, &address));
         CPPUNIT_ASSERT(address.isNull());

         const char *sdp2 =
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12\r\n"
            "t=2873397496 2873404696\r\n"
            "a=recvonly\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "m=video 51372 RTP/AVP 31\r\n"
            "c=IN IP4 224.2.17.15/127\r\n"
            "m=application 32416 udp wb\r\n"
            "a=orient:portrait\r\n"
            ;
         SdpBody body2(sdp2);

         CPPUNIT_ASSERT(body2.getMediaAddress(0, &address));
         ASSERT_STR_EQUAL( address.data(), "224.2.17.12");

         CPPUNIT_ASSERT(body2.getMediaAddress(1, &address));
         ASSERT_STR_EQUAL( address.data(), "224.2.17.15");

         CPPUNIT_ASSERT(body2.getMediaAddress(2, &address));
         ASSERT_STR_EQUAL( address.data(), "224.2.17.12");
      }

    void testGetMediaAttribute()
    {
       const char *sdp =
          "v=0\r\n"
          "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
          "s=SDP Seminar\r\n"
          "i=A Seminar on the session description protocol\r\n"
          "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
          "e=mjh@isi.edu (Mark Handley)\r\n"
          "c=IN IP4 224.2.17.12/127\r\n"
          "t=2873397496 2873404696\r\n"
          "a=test1\r\n"
          "a=test2:value2\r\n"
          "a=test3\r\n"
          "a=test4:value4\r\n"
          "a=test11:value11\r\n"
          "m=audio 49170 RTP/AVP 0\r\n"    // media description 0
          "a=test1\r\n"
          "a=test2:override_value2\r\n"
          "a=test5\r\n"
          "a=test6:value6\r\n"
          "a=test11:override_0_value11\r\n"
          "m=video 51372 RTP/AVP 31\r\n"   // media description 1
          "a=test3\r\n"
          "a=test4:override_value4\r\n"
          "a=test7\r\n"
          "a=test8:value8\r\n"
          "a=test11:override_1_value11\r\n"
          "m=application 32416 udp wb\r\n" // media description 2
          "a=test9\r\n"
          "a=test10:value10\r\n"
          "a=test11:override_2_value11\r\n"
          ;
       SdpBody body(sdp);
       UtlString attributeValue;

       CPPUNIT_ASSERT( body.getMediaAttribute( 0, "test1", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 0, "test2", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "override_value2", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 0, "test3", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 0, "test4", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "value4", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 0, "test5", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 0, "test6", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "value6", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 0, "test7", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 0, "test8", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 0, "test9", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 0, "test10", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 0, "test11", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "override_0_value11", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 0, "dummy", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );

       CPPUNIT_ASSERT( body.getMediaAttribute( 1, "test1", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 1, "test2", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "value2", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 1, "test3", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 1, "test4", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "override_value4", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 1, "test5", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 1, "test6", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 1, "test7", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 1, "test8", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "value8", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 1, "test9", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 1, "test10", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 1, "test11", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "override_1_value11", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 1, "dummy", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );

       CPPUNIT_ASSERT( body.getMediaAttribute( 2, "test1", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 2, "test2", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "value2", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 2, "test3", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 2, "test4", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "value4", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 2, "test5", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 2, "test6", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 2, "test7", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 2, "test8", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 2, "test9", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 2, "test10", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "value10", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 2, "test11", &attributeValue ) == TRUE );
       ASSERT_STR_EQUAL( "override_2_value11", attributeValue.data() );
       CPPUNIT_ASSERT( body.getMediaAttribute( 2, "dummy", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );

       CPPUNIT_ASSERT( body.getMediaAttribute( 3, "test1", &attributeValue ) == FALSE );
       ASSERT_STR_EQUAL( "", attributeValue.data() );
    }

    void testRemoveMediaAttribute()
    {
       const char *sdp =
          "v=0\r\n"
          "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
          "s=SDP Seminar\r\n"
          "i=A Seminar on the session description protocol\r\n"
          "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
          "e=mjh@isi.edu (Mark Handley)\r\n"
          "c=IN IP4 224.2.17.12/127\r\n"
          "t=2873397496 2873404696\r\n"
          "a=test0\r\n"
          "a=test1\r\n"
          "a=test2\r\n"
          "a=someattrib1:specialvalue1\r\n"
          "m=audio 49170 RTP/AVP 0\r\n"    // media description 0
          "a=test1\r\n"
          "a=test3\r\n"
          "a=test4\r\n"
          "a=someattrib2\r\n"
          "m=video 51372 RTP/AVP 31\r\n"   // media description 1
          "a=test5\r\n"
          "a=test6\r\n"
          "a=someattrib3:specialvalue3\r\n"
          "m=application 32416 udp wb\r\n" // media description 2
          "m=audio 32416 udp wb\r\n" // media description 3
          "a=test7\r\n"
          "a=test8\r\n"
          ;
       SdpBody body(sdp);

       CPPUNIT_ASSERT( body.removeMediaAttribute( 0, "test0" ) == true );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 0, "test1" ) == true );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 0, "test2" ) == true );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 0, "test3" ) == true );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 0, "test4" ) == true );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 0, "test5" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 0, "test6" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 0, "test7" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 0, "test8" ) == false );

       const char *expectedSdp1 =
          "v=0\r\n"
          "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
          "s=SDP Seminar\r\n"
          "i=A Seminar on the session description protocol\r\n"
          "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
          "e=mjh@isi.edu (Mark Handley)\r\n"
          "c=IN IP4 224.2.17.12/127\r\n"
          "t=2873397496 2873404696\r\n"
          "a=test1\r\n"
          "a=someattrib1:specialvalue1\r\n"
          "m=audio 49170 RTP/AVP 0\r\n"    // media description 0
          "a=someattrib2\r\n"
          "m=video 51372 RTP/AVP 31\r\n"   // media description 1
          "a=test5\r\n"
          "a=test6\r\n"
          "a=someattrib3:specialvalue3\r\n"
          "m=application 32416 udp wb\r\n" // media description 2
          "m=audio 32416 udp wb\r\n"       // media description 3
          "a=test7\r\n"
          "a=test8\r\n"
          ;
       SdpBody expectedBody1(expectedSdp1);

       UtlString testBodyText, expectedBodyText;
       ssize_t testLen, expectedLen;

       body.         getBytes( &testBodyText,     &testLen );
       expectedBody1.getBytes( &expectedBodyText, &expectedLen );

       ASSERT_STR_EQUAL( expectedBodyText.data(), testBodyText.data() );
       CPPUNIT_ASSERT( expectedLen == testLen );

       // media description index 1
       CPPUNIT_ASSERT( body.removeMediaAttribute( 1, "test0" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 1, "test1" ) == true );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 1, "test2" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 1, "test3" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 1, "test4" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 1, "test5" ) == true );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 1, "test6" ) == true );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 1, "test7" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 1, "test8" ) == false );

       const char *expectedSdp2 =
          "v=0\r\n"
          "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
          "s=SDP Seminar\r\n"
          "i=A Seminar on the session description protocol\r\n"
          "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
          "e=mjh@isi.edu (Mark Handley)\r\n"
          "c=IN IP4 224.2.17.12/127\r\n"
          "t=2873397496 2873404696\r\n"
          "a=someattrib1:specialvalue1\r\n"
          "m=audio 49170 RTP/AVP 0\r\n"    // media description 0
          "a=someattrib2\r\n"
          "m=video 51372 RTP/AVP 31\r\n"   // media description 1
          "a=someattrib3:specialvalue3\r\n"
          "m=application 32416 udp wb\r\n" // media description 2
          "m=audio 32416 udp wb\r\n"       // media description 3
          "a=test7\r\n"
          "a=test8\r\n"
          ;
       SdpBody expectedBody2(expectedSdp2);

       body.         getBytes( &testBodyText,     &testLen );
       expectedBody2.getBytes( &expectedBodyText, &expectedLen );

       ASSERT_STR_EQUAL( expectedBodyText.data(), testBodyText.data() );
       CPPUNIT_ASSERT( expectedLen == testLen );

       // media description index 2
       CPPUNIT_ASSERT( body.removeMediaAttribute( 2, "test0" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 2, "test1" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 2, "test2" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 2, "test3" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 2, "test4" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 2, "test5" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 2, "test6" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 2, "test7" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 2, "test8" ) == false );

       const char *expectedSdp3 =
          "v=0\r\n"
          "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
          "s=SDP Seminar\r\n"
          "i=A Seminar on the session description protocol\r\n"
          "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
          "e=mjh@isi.edu (Mark Handley)\r\n"
          "c=IN IP4 224.2.17.12/127\r\n"
          "t=2873397496 2873404696\r\n"
          "a=someattrib1:specialvalue1\r\n"
          "m=audio 49170 RTP/AVP 0\r\n"    // media description 0
          "a=someattrib2\r\n"
          "m=video 51372 RTP/AVP 31\r\n"   // media description 1
          "a=someattrib3:specialvalue3\r\n"
          "m=application 32416 udp wb\r\n" // media description 2
          "m=audio 32416 udp wb\r\n"       // media description 3
          "a=test7\r\n"
          "a=test8\r\n"
          ;
       SdpBody expectedBody3(expectedSdp3);

       body.         getBytes( &testBodyText,     &testLen );
       expectedBody3.getBytes( &expectedBodyText, &expectedLen );

       ASSERT_STR_EQUAL( expectedBodyText.data(), testBodyText.data() );
       CPPUNIT_ASSERT( expectedLen == testLen );

       // media description index 3
       CPPUNIT_ASSERT( body.removeMediaAttribute( 3, "test0" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 3, "test1" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 3, "test2" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 3, "test3" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 3, "test4" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 3, "test5" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 3, "test6" ) == false );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 3, "test7" ) == true );
       CPPUNIT_ASSERT( body.removeMediaAttribute( 3, "test8" ) == true );

       const char *expectedSdp4 =
          "v=0\r\n"
          "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
          "s=SDP Seminar\r\n"
          "i=A Seminar on the session description protocol\r\n"
          "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
          "e=mjh@isi.edu (Mark Handley)\r\n"
          "c=IN IP4 224.2.17.12/127\r\n"
          "t=2873397496 2873404696\r\n"
          "a=someattrib1:specialvalue1\r\n"
          "m=audio 49170 RTP/AVP 0\r\n"    // media description 0
          "a=someattrib2\r\n"
          "m=video 51372 RTP/AVP 31\r\n"   // media description 1
          "a=someattrib3:specialvalue3\r\n"
          "m=application 32416 udp wb\r\n" // media description 2
          "m=audio 32416 udp wb\r\n"       // media description 3
          ;
       SdpBody expectedBody4(expectedSdp4);

       body.         getBytes( &testBodyText,     &testLen );
       expectedBody4.getBytes( &expectedBodyText, &expectedLen );

       ASSERT_STR_EQUAL( expectedBodyText.data(), testBodyText.data() );
       CPPUNIT_ASSERT( expectedLen == testLen );
    }

    void testInsertAttribute()
    {
       const char *sdp =
          "v=0\r\n"
          "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
          "s=SDP Seminar\r\n"
          "i=A Seminar on the session description protocol\r\n"
          "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
          "e=mjh@isi.edu (Mark Handley)\r\n"
          "c=IN IP4 224.2.17.12/127\r\n"
          "t=2873397496 2873404696\r\n"
          "a=test1\r\n"
          "a=test2\r\n"
          "m=audio 49170 RTP/AVP 0\r\n"    // media description 0
          "a=test1\r\n"
          "a=test2\r\n"
          "m=video 51372 RTP/AVP 31\r\n"   // media description 1
          "a=test1\r\n"
          "a=test2\r\n"
          "m=application 32416 udp wb\r\n" // media description 2
          "a=test1\r\n"
          "a=test2\r\n"
          ;
       SdpBody body(sdp);

       CPPUNIT_ASSERT( body.insertMediaAttribute( -1, "someattrib1", "specialvalue1" ) == true );
       CPPUNIT_ASSERT( body.insertMediaAttribute(  0, "someattrib2" ) == true );
       CPPUNIT_ASSERT( body.insertMediaAttribute(  1, "someattrib3", "specialvalue3" ) == true );
       CPPUNIT_ASSERT( body.insertMediaAttribute(  2, "someattrib4", "specialvalue4" ) == true );
       CPPUNIT_ASSERT( body.insertMediaAttribute(  3, "someattrib5", "specialvalue5" ) == false );
       CPPUNIT_ASSERT( body.insertMediaAttribute(  3, 0 ) == false );

       const char *expectedSdp =
          "v=0\r\n"
          "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
          "s=SDP Seminar\r\n"
          "i=A Seminar on the session description protocol\r\n"
          "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
          "e=mjh@isi.edu (Mark Handley)\r\n"
          "c=IN IP4 224.2.17.12/127\r\n"
          "t=2873397496 2873404696\r\n"
          "a=test1\r\n"
          "a=test2\r\n"
          "a=someattrib1:specialvalue1\r\n"
          "m=audio 49170 RTP/AVP 0\r\n"    // media description 0
          "a=test1\r\n"
          "a=test2\r\n"
          "a=someattrib2\r\n"
          "m=video 51372 RTP/AVP 31\r\n"   // media description 1
          "a=test1\r\n"
          "a=test2\r\n"
          "a=someattrib3:specialvalue3\r\n"
          "m=application 32416 udp wb\r\n" // media description 2
          "a=test1\r\n"
          "a=test2\r\n"
          "a=someattrib4:specialvalue4\r\n"
          ;
       SdpBody expectedBody(expectedSdp);

       UtlString testBodyText, expectedBodyText;
       ssize_t testLen, expectedLen;

       body.        getBytes( &testBodyText,     &testLen );
       expectedBody.getBytes( &expectedBodyText, &expectedLen );

       ASSERT_STR_EQUAL( expectedBodyText.data(), testBodyText.data() );
       CPPUNIT_ASSERT( expectedLen == testLen );
    }

    void testModifyMediaAddress()
    {
       const char *sdp =
          "v=0\r\n"
          "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
          "s=SDP Seminar\r\n"
          "i=A Seminar on the session description protocol\r\n"
          "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
          "e=mjh@isi.edu (Mark Handley)\r\n"
          "t=2873397496 2873404696\r\n"
          "a=test1\r\n"
          "a=test2\r\n"
          "m=audio 49170 RTP/AVP 0\r\n"    // media description 0
          "c=IN IP4 224.2.17.12/127\r\n"
          "a=test1\r\n"
          "a=test2\r\n"
          "m=video 51372 RTP/AVP 31\r\n"   // media description 1
          "m=application 32416 udp wb\r\n" // media description 2
          "i=test\r\n"
          "m=audio 55554 RTP/AVP 0\r\n" // media description 3
          "i=test\r\n"
          "c=IN IP4 224.2.17.12/127\r\n"
          "a=test1\r\n"
          "a=test2\r\n"
          "m=audio 55560 RTP/AVP 0\r\n" // media description 4
          "m=audio 55562 RTP/AVP 0\r\n" // media description 5
          "b=dummy\r\n"
          "m=audio 55564 RTP/AVP 0\r\n" // media description 6
          "z=dummy\r\n"
          "m=audio 55566 RTP/AVP 0\r\n" // media description 7
          "k=dummy\r\n"
          "m=audio 55568 RTP/AVP 0\r\n" // media description 8
          "a=dummy\r\n"
          "m=audio 55510 RTP/AVP 0\r\n" // media description 9
          "t=dummy\r\n"
          "m=audio 55520 RTP/AVP 0\r\n" // media description 10
          "r=dummy\r\n"
          "m=audio 55530 RTP/AVP 0\r\n" // media description 11
          "m=audio 55534 RTP/AVP 0\r\n" // media description 12
          ;
       SdpBody body(sdp);

       CPPUNIT_ASSERT( body.modifyMediaAddress( -1, "10.10.10.0" ) == true );
       CPPUNIT_ASSERT( body.modifyMediaAddress(  0, "10.10.10.1" ) == true );
       CPPUNIT_ASSERT( body.modifyMediaAddress(  1, "10.10.10.2" ) == true );
       CPPUNIT_ASSERT( body.modifyMediaAddress(  2, "10.10.10.3" ) == true );
       CPPUNIT_ASSERT( body.modifyMediaAddress(  3, "10.10.10.4" ) == true );
       CPPUNIT_ASSERT( body.modifyMediaAddress(  4, "10.10.10.5" ) == true );
       CPPUNIT_ASSERT( body.modifyMediaAddress(  5, "10.10.10.6" ) == true );
       CPPUNIT_ASSERT( body.modifyMediaAddress(  6, "10.10.10.7" ) == true );
       CPPUNIT_ASSERT( body.modifyMediaAddress(  7, "10.10.10.8" ) == true );
       CPPUNIT_ASSERT( body.modifyMediaAddress(  8, "10.10.10.9" ) == true );
       CPPUNIT_ASSERT( body.modifyMediaAddress(  9, "10.10.10.10" ) == true );
       CPPUNIT_ASSERT( body.modifyMediaAddress( 10, "10.10.10.11" ) == true );
       CPPUNIT_ASSERT( body.modifyMediaAddress( 11, "10.10.10.12" ) == true );
       CPPUNIT_ASSERT( body.modifyMediaAddress( 12, "10.10.10.13" ) == true );
       CPPUNIT_ASSERT( body.modifyMediaAddress( 13, "10.10.10.14" ) == false );
       CPPUNIT_ASSERT( body.modifyMediaAddress( 1, 0 ) == false );

       const char *expectedSdp =
          "v=0\r\n"
          "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
          "s=SDP Seminar\r\n"
          "i=A Seminar on the session description protocol\r\n"
          "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
          "e=mjh@isi.edu (Mark Handley)\r\n"
          "c=IN IP4 10.10.10.0\r\n"
          "t=2873397496 2873404696\r\n"
          "a=test1\r\n"
          "a=test2\r\n"
          "m=audio 49170 RTP/AVP 0\r\n"    // media description 0
          "c=IN IP4 10.10.10.1\r\n"
          "a=test1\r\n"
          "a=test2\r\n"
          "m=video 51372 RTP/AVP 31\r\n"   // media description 1
          "c=IN IP4 10.10.10.2\r\n"
          "m=application 32416 udp wb\r\n" // media description 2
          "i=test\r\n"
          "c=IN IP4 10.10.10.3\r\n"
          "m=audio 55554 RTP/AVP 0\r\n" // media description 3
          "i=test\r\n"
          "c=IN IP4 10.10.10.4\r\n"
          "a=test1\r\n"
          "a=test2\r\n"
          "m=audio 55560 RTP/AVP 0\r\n" // media description 4
          "c=IN IP4 10.10.10.5\r\n"
          "m=audio 55562 RTP/AVP 0\r\n" // media description 5
          "c=IN IP4 10.10.10.6\r\n"
          "b=dummy\r\n"
          "m=audio 55564 RTP/AVP 0\r\n" // media description 6
          "c=IN IP4 10.10.10.7\r\n"
          "z=dummy\r\n"
          "m=audio 55566 RTP/AVP 0\r\n" // media description 7
          "c=IN IP4 10.10.10.8\r\n"
          "k=dummy\r\n"
          "m=audio 55568 RTP/AVP 0\r\n" // media description 8
          "c=IN IP4 10.10.10.9\r\n"
          "a=dummy\r\n"
          "m=audio 55510 RTP/AVP 0\r\n" // media description 9
          "c=IN IP4 10.10.10.10\r\n"
          "t=dummy\r\n"
          "m=audio 55520 RTP/AVP 0\r\n" // media description 10
          "c=IN IP4 10.10.10.11\r\n"
          "r=dummy\r\n"
          "m=audio 55530 RTP/AVP 0\r\n" // media description 11
          "c=IN IP4 10.10.10.12\r\n"
          "m=audio 55534 RTP/AVP 0\r\n" // media description 12
          "c=IN IP4 10.10.10.13\r\n"
          ;
       SdpBody expectedBody(expectedSdp);

       UtlString testBodyText, expectedBodyText;
       ssize_t testLen, expectedLen;

       body.        getBytes( &testBodyText,     &testLen );
       expectedBody.getBytes( &expectedBodyText, &expectedLen );

       ASSERT_STR_EQUAL( expectedBodyText.data(), testBodyText.data() );
       CPPUNIT_ASSERT( expectedLen == testLen );
    }

    void testModifyMediaPort()
    {
       const char *sdp =
          "v=0\r\n"
          "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
          "s=SDP Seminar\r\n"
          "i=A Seminar on the session description protocol\r\n"
          "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
          "e=mjh@isi.edu (Mark Handley)\r\n"
          "c=IN IP4 224.2.17.12/127\r\n"
          "t=2873397496 2873404696\r\n"
          "a=test1\r\n"
          "a=test2\r\n"
          "m=audio 49170 RTP/AVP 0\r\n"    // media description 0
          "c=IN IP4 224.2.17.12/127\r\n"
          "a=test1\r\n"
          "a=test2\r\n"
          "m=video 51372 RTP/AVP 31\r\n"   // media description 1
          "m=application 32416 udp wb\r\n" // media description 2
          "i=test\r\n"
          "m=audio 55554 RTP/AVP 0\r\n" // media description 3
          "i=test\r\n"
          "c=IN IP4 224.2.17.12/127\r\n"
          "a=test1\r\n"
          "a=test2\r\n"
          "m=audio 55560 RTP/AVP 0\r\n" // media description 4
          "a=test1\r\n"
          "a=test2\r\n"
          ;
       SdpBody body(sdp);

       CPPUNIT_ASSERT( body.modifyMediaPort( 0, 10000 ) == true );
       CPPUNIT_ASSERT( body.modifyMediaPort( 1, 11000 ) == true );
       CPPUNIT_ASSERT( body.modifyMediaPort( 2, 12000 ) == true );
       CPPUNIT_ASSERT( body.modifyMediaPort( 3, 13000 ) == true );
       CPPUNIT_ASSERT( body.modifyMediaPort( 4, 14000 ) == true );
       CPPUNIT_ASSERT( body.modifyMediaPort( 5, 15000 ) == false );

       const char *expectedSdp =
          "v=0\r\n"
          "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
          "s=SDP Seminar\r\n"
          "i=A Seminar on the session description protocol\r\n"
          "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
          "e=mjh@isi.edu (Mark Handley)\r\n"
          "c=IN IP4 224.2.17.12/127\r\n"
          "t=2873397496 2873404696\r\n"
          "a=test1\r\n"
          "a=test2\r\n"
          "m=audio 10000 RTP/AVP 0\r\n"    // media description 0
          "c=IN IP4 224.2.17.12/127\r\n"
          "a=test1\r\n"
          "a=test2\r\n"
          "m=video 11000 RTP/AVP 31\r\n"   // media description 1
          "m=application 12000 udp wb\r\n" // media description 2
          "i=test\r\n"
          "m=audio 13000 RTP/AVP 0\r\n" // media description 3
          "i=test\r\n"
          "c=IN IP4 224.2.17.12/127\r\n"
          "a=test1\r\n"
          "a=test2\r\n"
          "m=audio 14000 RTP/AVP 0\r\n" // media description 4
          "a=test1\r\n"
          "a=test2\r\n"
          ;
       SdpBody expectedBody(expectedSdp);

       UtlString testBodyText, expectedBodyText;
       ssize_t testLen, expectedLen;

       body.        getBytes( &testBodyText,     &testLen );
       expectedBody.getBytes( &expectedBodyText, &expectedLen );

       ASSERT_STR_EQUAL( expectedBodyText.data(), testBodyText.data() );
       CPPUNIT_ASSERT( expectedLen == testLen );
    }

    void testCandidateParsing()
    {
        UtlString id ;
        double qValue ;
        UtlString userFrag ;
        UtlString password ;
        UtlString ip ;
        int port ;
        UtlString candidateIp ;
        int candidatePort ;
        UtlBoolean bRC ;
        int i;

        /*
         * First test: Verify that we don't blow up when no candidate
         * attributes are found.
         */
        const char* sdp1 =
            "v=0\r\n"
            "o=Pingtel 5 5 IN IP4 192.168.1.102\r\n"
            "s=phone-call\r\n"
            "c=IN IP4 192.168.1.102\r\n"
            "t=0 0\r\n"
            "m=audio 8778 RTP/AVP 0 8 96\r\n"
            "a=rtpmap:0 pcmu/8000/1\r\n"
            "a=rtpmap:8 pcma/8000/1\r\n"
            "a=rtpmap:96 telephone-event/8000/1\r\n" ;

        SdpBody body1(sdp1) ;

        for (i=-1; i<4; i++)
        {
            bRC = body1.getCandidateAttribute(-1, id, qValue, userFrag,
                    password, ip, port, candidateIp, candidatePort) ;
            CPPUNIT_ASSERT(bRC == FALSE) ;
        }

        /*
         * Second test: Verify that we can parse expected data
         */
        const char* sdp2 =
            "v=0\r\n"
            "o=Pingtel 5 5 IN IP4 192.168.1.102\r\n"
            "s=phone-call\r\n"
            "c=IN IP4 192.168.1.102\r\n"
            "t=0 0\r\n"
            "m=audio 8778 RTP/AVP 0 8 96\r\n"
            "a=rtpmap:0 pcmu/8000/1\r\n"
            "a=rtpmap:8 pcma/8000/1\r\n"
            "a=rtpmap:96 telephone-event/8000/1\r\n"
            "a=candidate:id 0.4 userfrag password 192.168.1.102 8878 10.1.1.102 9999\r\n"
            "a=candidate:id 0.5 userfrag2 password2 192.168.1.102 8878 10.1.1.102 10000\r\n"
            "a=candidate:id 0.6 userfrag3 password3 192.168.1.102 8878 10.1.1.103 9999\r\n" ;

        SdpBody body2(sdp2) ;

        bRC = body2.getCandidateAttribute(-1, id, qValue, userFrag,
                password, ip, port, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == FALSE) ;

        bRC = body2.getCandidateAttribute(0, id, qValue, userFrag,
                password, ip, port, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        ASSERT_STR_EQUAL("id", id.data()) ;
        CPPUNIT_ASSERT(qValue == 0.4) ;
        ASSERT_STR_EQUAL("userfrag", userFrag.data()) ;
        ASSERT_STR_EQUAL("password", password.data()) ;
        ASSERT_STR_EQUAL("192.168.1.102", ip.data()) ;
        CPPUNIT_ASSERT(port == 8878) ;
        ASSERT_STR_EQUAL("10.1.1.102", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 9999) ;

        bRC = body2.getCandidateAttribute(1, id, qValue, userFrag,
                password, ip, port, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        ASSERT_STR_EQUAL("id", id.data()) ;
        CPPUNIT_ASSERT(qValue == 0.5) ;
        ASSERT_STR_EQUAL("userfrag2", userFrag.data()) ;
        ASSERT_STR_EQUAL("password2", password.data()) ;
        ASSERT_STR_EQUAL("192.168.1.102", ip.data()) ;
        CPPUNIT_ASSERT(port == 8878) ;
        ASSERT_STR_EQUAL("10.1.1.102", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 10000) ;

        bRC = body2.getCandidateAttribute(2, id, qValue, userFrag,
                password, ip, port, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == TRUE) ;
        ASSERT_STR_EQUAL("id", id.data()) ;
        CPPUNIT_ASSERT(qValue == 0.6) ;
        ASSERT_STR_EQUAL("userfrag3", userFrag.data()) ;
        ASSERT_STR_EQUAL("password3", password.data()) ;
        ASSERT_STR_EQUAL("192.168.1.102", ip.data()) ;
        CPPUNIT_ASSERT(port == 8878) ;
        ASSERT_STR_EQUAL("10.1.1.103", candidateIp.data()) ;
        CPPUNIT_ASSERT(candidatePort == 9999) ;

        bRC = body2.getCandidateAttribute(3, id, qValue, userFrag,
                password, ip, port, candidateIp, candidatePort) ;
        CPPUNIT_ASSERT(bRC == FALSE) ;

        SdpBody testBody;
        const char* testBodyExpected =
                "v=0\r\n"
                "o=sipXecs 5 5 IN IP4 127.0.0.1\r\n"
                "s=\r\n"
                "a=candidate:id1 0.2 userfrag password 192.168.1.102 8776 10.1.1.102 9999\r\n"
                "a=candidate:id2 0.3 userfrag2 password2 192.168.1.103 8777 10.1.1.103 10000\r\n" ;

        testBody.addCandidateAttribute("id1", 0.2, "userfrag", "password",
                "192.168.1.102", 8776, "10.1.1.102", 9999) ;
        testBody.addCandidateAttribute("id2", 0.3, "userfrag2", "password2",
                "192.168.1.103", 8777, "10.1.1.103", 10000) ;
        UtlString strBody ;
        ssize_t nBody = 0;
        testBody.getBytes(&strBody, &nBody) ;

        ASSERT_STR_EQUAL(testBodyExpected, strBody.data()) ;
    }

    void testRtcpPortParsing()
    {
        SdpBody testBody ;
        SdpSrtpParameters testSrtp;
        UtlString strBody ;
        ssize_t nBody ;

        testSrtp.securityLevel = 0;

        SdpCodec* pAudioCodec = new SdpCodec(SdpCodec::SDP_CODEC_PCMU, 99, "audio", "superaudio") ;
        SdpCodec* pVideoCodec = new SdpCodec(SdpCodec::SDP_CODEC_PCMU, 100, "video", "supervideo") ;
        SdpCodec* pAppCodec = new SdpCodec(SdpCodec::SDP_CODEC_PCMU, 101, "app", "superapp") ;

        // This test case isn't exactly valid, but allows us to walk the m lines.
        testBody.setSessionNameField("foo") ;
        testBody.addAudioCodecs("10.1.1.30", 8700, 8701, 0, 0, 1, &pAudioCodec, testSrtp) ;
        testBody.addAudioCodecs("10.1.1.31", 0, 0, 8801, 8802, 1, &pVideoCodec, testSrtp) ;
        testBody.addAudioCodecs("10.1.1.32", 8900, 8999, 0, 0, 1, &pAppCodec, testSrtp) ;

        const char* testBodyExpected =
            "v=0\r\n"
            "o=sipXecs 5 5 IN IP4 127.0.0.1\r\n"
            "s=foo\r\n"
            "c=IN IP4 10.1.1.30\r\n"
            "t=0 0\r\n"
            "m=audio 8700 RTP/AVP 99\r\n"
            "a=rtpmap:99 superaudio/8000/1\r\n"
            "m=video 8801 RTP/AVP 100\r\n"
            "a=rtcp:8802\r\n"
            "a=rtpmap:100 supervideo/8000/1\r\n"
            "a=fmtp:100 size:QCIF\r\n"
            "c=IN IP4 10.1.1.31\r\n"
            "m=audio 8900 RTP/AVP 101\r\n"
            "a=rtcp:8999\r\n"
            "a=rtpmap:101 superapp/8000/1\r\n"
            "c=IN IP4 10.1.1.32\r\n" ;

        UtlString address ;
        int port ;

        testBody.getBytes(&strBody, &nBody) ;
        ASSERT_STR_EQUAL(testBodyExpected, strBody.data()) ;


        SdpBody bodyCheck(testBodyExpected) ;

        CPPUNIT_ASSERT(bodyCheck.getMediaSetCount() == 3) ;

        bodyCheck.getMediaAddress(0, &address) ;
        ASSERT_STR_EQUAL("10.1.1.30", address.data()) ;
        bodyCheck.getMediaPort(0, &port) ;
        CPPUNIT_ASSERT(port == 8700) ;
        bodyCheck.getMediaRtcpPort(0, &port) ;
        CPPUNIT_ASSERT(port == 8701) ;

        bodyCheck.getMediaAddress(1, &address) ;
        ASSERT_STR_EQUAL("10.1.1.31", address.data()) ;
        bodyCheck.getMediaPort(1, &port) ;
        CPPUNIT_ASSERT(port == 8801) ;
        bodyCheck.getMediaRtcpPort(1, &port) ;
        CPPUNIT_ASSERT(port == 8802) ;

        bodyCheck.getMediaAddress(2, &address) ;
        ASSERT_STR_EQUAL("10.1.1.32", address.data()) ;
        bodyCheck.getMediaPort(2, &port) ;
        CPPUNIT_ASSERT(port == 8900) ;
        bodyCheck.getMediaRtcpPort(2, &port) ;
        CPPUNIT_ASSERT(port == 8999) ;
    }

    void testAddAttribute()
    {
        const char *sdp =
           "v=0\r\n"
           "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
           "s=SDP Seminar\r\n"
           "i=A Seminar on the session description protocol\r\n"
           "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
           "e=mjh@isi.edu (Mark Handley)\r\n"
           "c=IN IP4 224.2.17.12/127\r\n"
           "t=2873397496 2873404696\r\n"
           "m=audio 49170 RTP/AVP 0\r\n"
           "m=video 51372 RTP/AVP 31\r\n"
           "m=application 32416 udp wb\r\n"
           ;
        SdpBody body(sdp);

        body.addAttribute("recvonly");
        body.addAttribute("orient", "portrait");

        const char *sdpExpected =
           "v=0\r\n"
           "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
           "s=SDP Seminar\r\n"
           "i=A Seminar on the session description protocol\r\n"
           "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
           "e=mjh@isi.edu (Mark Handley)\r\n"
           "c=IN IP4 224.2.17.12/127\r\n"
           "t=2873397496 2873404696\r\n"
           "m=audio 49170 RTP/AVP 0\r\n"
           "m=video 51372 RTP/AVP 31\r\n"
           "m=application 32416 udp wb\r\n"
           "a=recvonly\r\n"
           "a=orient:portrait\r\n"
           ;

        UtlString strBody ;
        ssize_t nBody ;
        body.getBytes(&strBody, &nBody) ;
        ASSERT_STR_EQUAL(sdpExpected, strBody.data()) ;
    }


    void testInsertTelephoneEvent()
    {
        int audioPayloadIdCount;
        int videoPayloadIdCount;
        int audioPayloadTypes[2];
        int videoPayloadTypes[2];

        int numLocalCodecs;
        SdpCodecFactory localRtpCodecs;
        UtlString codecList("");

        int numCodecsInCommon;
        int& rnumCodecsInCommon = numCodecsInCommon;

        SdpCodec** ppmatchingCodecs = NULL;
        SdpCodec**& rppcodecsInCommonArray = ppmatchingCodecs;
        rppcodecsInCommonArray = new SdpCodec*[4];

        numCodecsInCommon = 0;
        audioPayloadIdCount = 1;
        videoPayloadIdCount = 1;
        audioPayloadTypes[0] = 8;
        videoPayloadTypes[0] = 101;
        numCodecsInCommon = 0;

        codecList.append("telephone-event ");
        codecList.append("pcmu ");
        codecList.append("pcma ");
        codecList.append("VP71-CIF ");

        localRtpCodecs.buildSdpCodecFactory(codecList);


        numLocalCodecs = 4;
//        memset(codecsInCommonArray, 0, sizeof(SdpCodec*) * 4);

        const char *sdp =
           "v=0\r\n"
           "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
           "s=SDP Seminar\r\n"
           "i=A Seminar on the session description protocol\r\n"
           "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
           "e=mjh@isi.edu (Mark Handley)\r\n"
           "c=IN IP4 224.2.17.12/127\r\n"
           "t=2873397496 2873404696\r\n"
           "m=audio 49170 RTP/AVP 8\r\n"
           "a=rtpmap:8 pcma/8000/1\r\n"
           "m=video 51372 RTP/AVP 101\r\n"
           "a=rtpmap:101 VP71-CIF/9000/2000/1\r\n"
           ;

        SdpBody body(sdp);

        body.getCodecsInCommon(1, 1,
                               &audioPayloadTypes[0],
                               &videoPayloadTypes[0],
                               localRtpCodecs,
                               rnumCodecsInCommon,
                               rppcodecsInCommonArray);
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SdpBodyTest__testInsertTelephoneEvent "
                       "numCodecs %d ",
                       numCodecsInCommon);

         CPPUNIT_ASSERT(numCodecsInCommon == 2) ;

         SdpCodec* pmatchingCodecs = *ppmatchingCodecs;
/*         SdpCodec matchingCodec1, matchingCodec2;

         matchingCodec1 = *pmatchingCodecs++;
         matchingCodec2 = *pmatchingCodecs++;
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SdpBodyTest__testInsertTelephoneEvent "
                       "pt[0] is %d "
                       "pt[1] is %d ",
                       matchingCodec1.getCodecPayloadFormat(),
                       matchingCodec2.getCodecPayloadFormat()); */

        int checkPT;
        checkPT = pmatchingCodecs->getCodecPayloadFormat();
        CPPUNIT_ASSERT(checkPT == 8) ;
/*
        pmatchingCodecs++;
        checkPT = pmatchingCodecs->getCodecPayloadFormat(); */
//        CPPUNIT_ASSERT(checkPT == 96) ;
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SdpBodyTest);
