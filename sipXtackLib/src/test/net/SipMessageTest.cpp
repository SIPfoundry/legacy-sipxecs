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
#include <utl/UtlHashMap.h>

#include <os/OsDefs.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>

#include <stdio.h>

/**
 * Unittest for SipMessage
 */
class SipMessageTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SipMessageTest);
      CPPUNIT_TEST(testCopyConstructor);
      CPPUNIT_TEST(testGetVia);
      CPPUNIT_TEST(testGetViaShort);
      CPPUNIT_TEST(testGetAddrVia);
      CPPUNIT_TEST(testGetNoBranchVia);
      CPPUNIT_TEST(testGetViaPort);
      CPPUNIT_TEST(testGetViaFieldSubField);
      CPPUNIT_TEST(testGetAllowEventField);
      CPPUNIT_TEST(testSetAllowEventField);
      CPPUNIT_TEST(testGetEventField);
      CPPUNIT_TEST(testGetEventFieldId);
      CPPUNIT_TEST(testGetEventFieldWithIdSpace);
      CPPUNIT_TEST(testGetEventFieldWithEqualSpace);
      CPPUNIT_TEST(testGetEventFieldWithTrailingSpace);
      CPPUNIT_TEST(testGetEventFieldWithParams);
      CPPUNIT_TEST(testGetEventFieldWithParamButNoValue);
      CPPUNIT_TEST(testGetToAddress);
      CPPUNIT_TEST(testGetFromAddress);
      CPPUNIT_TEST(testGetResponseSendAddress);
      CPPUNIT_TEST(testParseAddressFromUriPort);
      CPPUNIT_TEST(testProbPort);
      CPPUNIT_TEST(testMultipartBody);
      CPPUNIT_TEST(testCodecError);
      CPPUNIT_TEST(testSdpParse);
      CPPUNIT_TEST(testNonSdpSipMessage);
      CPPUNIT_TEST(testSetInviteDataHeaders);
      CPPUNIT_TEST(testSetInviteDataHeadersUnique);
      CPPUNIT_TEST(testSetInviteDataHeadersForbidden);
      CPPUNIT_TEST(testCompactNames);
      CPPUNIT_TEST(testApplyTargetUriHeaderParams);
      CPPUNIT_TEST(testGetContactUri);
      CPPUNIT_TEST(testGetFieldUris);
      CPPUNIT_TEST(testBuildSipUri);
      CPPUNIT_TEST(testReplacesData);
      CPPUNIT_TEST(testIsClientStrictRouted);
      CPPUNIT_TEST(testRecordRoutesAccepted);
      CPPUNIT_TEST(testPathFieldManipulations);
      CPPUNIT_TEST(testRouteFieldManipulations);
      CPPUNIT_TEST(test200ResponseToRequestWithPath);
      CPPUNIT_TEST(testSipXNatRoute);
      CPPUNIT_TEST(testGetFieldSubfield);
      CPPUNIT_TEST(testSetFieldSubfield_SingleSubfields);
      CPPUNIT_TEST(testSetFieldSubfield_Mixed);
      CPPUNIT_TEST(testSetViaTag);
      CPPUNIT_TEST(testRecordRouteEchoing);
      CPPUNIT_TEST(testDialogMatching);
      CPPUNIT_TEST(testGetReferencesField);
      CPPUNIT_TEST(testGetCSeqField);
      CPPUNIT_TEST_SUITE_END();

      public:

   void testCopyConstructor()
      {
         const char* SimpleMessage =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

         UtlString msgBytes;
         ssize_t   msgLength;

         testMsg.getBytes(&msgBytes, &msgLength);
         ASSERT_STR_EQUAL(SimpleMessage, msgBytes.data());

         SipMessage copiedMsg(testMsg);

         copiedMsg.getBytes(&msgBytes, &msgLength);
         ASSERT_STR_EQUAL(SimpleMessage, msgBytes.data());
      }

   void testGetVia()
      {
         const char* SimpleMessage =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

         UtlString viaAddress;
         int viaPort;
         UtlString protocol;
         int recievedPort;
         UtlBoolean receivedSet;
         UtlBoolean maddrSet;
         UtlBoolean receivePortSet;

         testMsg.getTopVia(&viaAddress,
                           &viaPort,
                           &protocol,
                           &recievedPort,
                           &receivedSet,
                           &maddrSet,
                           &receivePortSet);

         ASSERT_STR_EQUAL("sipx.local",viaAddress.data());
         CPPUNIT_ASSERT_EQUAL(33855, viaPort);
         ASSERT_STR_EQUAL("TCP",protocol.data());

      };

   void testGetViaShort()
      {
         const char* SimpleMessage =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "v: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-9378a12d4218e10ef4dc78ea3d\r\n"
            "v: SIP/2.0/UDP sipx.remote:9999;branch=z9hG4bK-10cb6f\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

         UtlString viaAddress;
         int viaPort;
         UtlString protocol;
         int recievedPort;
         UtlBoolean receivedSet;
         UtlBoolean maddrSet;
         UtlBoolean receivePortSet;

         testMsg.removeTopVia();

         testMsg.getTopVia(&viaAddress,
                           &viaPort,
                           &protocol,
                           &recievedPort,
                           &receivedSet,
                           &maddrSet,
                           &receivePortSet);

         ASSERT_STR_EQUAL("sipx.remote",viaAddress.data());
         CPPUNIT_ASSERT_EQUAL(9999, viaPort);
         ASSERT_STR_EQUAL("UDP",protocol.data());

      };


   void testGetAddrVia()
      {
         const char* SimpleMessage =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

         UtlString viaAddress;
         int viaPort;
         UtlString protocol;
         int recievedPort;
         UtlBoolean receivedSet;
         UtlBoolean maddrSet;
         UtlBoolean receivePortSet;

         testMsg.getTopVia(&viaAddress,
                           &viaPort,
                           &protocol,
                           &recievedPort,
                           &receivedSet,
                           &maddrSet,
                           &receivePortSet);

         ASSERT_STR_EQUAL("10.1.1.3",viaAddress.data());
         CPPUNIT_ASSERT_EQUAL(33855, viaPort);
         ASSERT_STR_EQUAL("TCP",protocol.data());

      };

   void testGetNoBranchVia()
      {
         const char* SimpleMessage =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

         UtlString viaAddress;
         int viaPort;
         UtlString protocol;
         int recievedPort;
         UtlBoolean receivedSet;
         UtlBoolean maddrSet;
         UtlBoolean receivePortSet;

         testMsg.getTopVia(&viaAddress,
                           &viaPort,
                           &protocol,
                           &recievedPort,
                           &receivedSet,
                           &maddrSet,
                           &receivePortSet);

         ASSERT_STR_EQUAL("10.1.1.3",viaAddress.data());
         CPPUNIT_ASSERT_EQUAL(33855, viaPort);
         ASSERT_STR_EQUAL("TCP",protocol.data());

      };

   void testGetViaPort()
      {
         struct test {
	   const char* string;	// Input string.
	   int port;		// Expected returned viaPort.
	   int rportSet;	// Expected returned receivedPortSet.
	   int rport;		// Expected returned receivedPort.
         };

	 struct test tests[] = {
	   { "sip:foo@bar", PORT_NONE, 0, PORT_NONE },
	   { "sip:foo@bar:5060", 5060, 0, PORT_NONE },
	   { "sip:foo@bar:1", 1, 0, PORT_NONE },
	   { "sip:foo@bar:100", 100, 0, PORT_NONE },
	   { "sip:foo@bar:65535", 65535, 0, PORT_NONE },
	   { "sip:foo@bar;rport=1", PORT_NONE, 1, 1 },
	   { "sip:foo@bar:5060;rport=1", 5060, 1, 1 },
	   { "sip:foo@bar:1;rport=1", 1, 1, 1 },
	   { "sip:foo@bar:100;rport=1", 100, 1, 1 },
	   { "sip:foo@bar:65535;rport=1", 65535, 1, 1 },
	   { "sip:foo@bar;rport=100", PORT_NONE, 1, 100 },
	   { "sip:foo@bar:5060;rport=100", 5060, 1, 100 },
	   { "sip:foo@bar:1;rport=100", 1, 1, 100 },
	   { "sip:foo@bar:100;rport=100", 100, 1, 100 },
	   { "sip:foo@bar:65535;rport=100", 65535, 1, 100 },
	   { "sip:foo@bar;rport=5060", PORT_NONE, 1, 5060 },
	   { "sip:foo@bar:5060;rport=5060", 5060, 1, 5060 },
	   { "sip:foo@bar:1;rport=5060", 1, 1, 5060 },
	   { "sip:foo@bar:100;rport=5060", 100, 1, 5060 },
	   { "sip:foo@bar:65535;rport=5060", 65535, 1, 5060 },
	   { "sip:foo@bar;rport=65535", PORT_NONE, 1, 65535 },
	   { "sip:foo@bar:5060;rport=65535", 5060, 1, 65535 },
	   { "sip:foo@bar:1;rport=65535", 1, 1, 65535 },
	   { "sip:foo@bar:100;rport=65535", 100, 1, 65535 },
	   { "sip:foo@bar:65535;rport=65535", 65535, 1, 65535 },
         };

         // Buffer to compose message.
         char message[1000];

         // Message templates into which to insert addresses.
         // Template has at least 2 Via's, to make sure the function is looking
         // at the right Via.
         const char* message_template =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP %s;branch=z9hG4bK-foobarbazquux\r\n"
            "Via: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         UtlString viaAddress;
         int viaPort;
         UtlString protocol;
         int receivedPort;
         UtlBoolean receivedSet;
         UtlBoolean maddrSet;
         UtlBoolean receivedPortSet;

         for (unsigned int i = 0; i < sizeof (tests) / sizeof (tests[0]);
              i++)
         {
            // Compose the message.
            sprintf(message, message_template, tests[i].string);
            SipMessage sipMessage(message, strlen(message));

            sipMessage.getTopVia(&viaAddress,
                                 &viaPort,
                                 &protocol,
                                 &receivedPort,
                                 &receivedSet,
                                 &maddrSet,
                                 &receivedPortSet);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(tests[i].string, tests[i].port,
					 viaPort);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(tests[i].string, tests[i].rportSet,
                                         receivedPortSet);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(tests[i].string, tests[i].rport,
                                         receivedPort);
         }
      }

   void testMultipartBody()
      {
         const char* MultipartBodyMessage =
            "INVITE sip:65681@testserver.com SIP/2.0\r\n"
            "Record-Route: <sip:172.20.26.36:5080;lr;a;t=15039611-4B5;s=5fc190d408cc09d245d115792f6d61e1>\r\n"
            "Via: SIP/2.0/UDP 172.20.26.36:5080;branch=z9hG4bK-826b994f3fa1136ea6da35868d05fcbb\r\n"
            "Via: SIP/2.0/TCP 172.20.26.36;branch=z9hG4bK-a45e8e1a92501d6d29854307651741b3\r\n"
            "Via: SIP/2.0/UDP  10.21.128.204:5060;branch=z9hG4bK9E8\r\n"
            "From: <sip:10.21.128.204>;tag=15039611-4B5\r\n"
            "To: <sip:65681@testserver.com>\r\n"
            "Date: Mon, 18 Jul 2005 18:05:17 GMT\r\n"
            "Call-Id: 55147C1E-F6ED11D9-80E3EC05-47D61469@10.21.128.204\r\n"
            "Supported: 100rel,timer\r\n"
            "Min-Se: 1800\r\n"
            "Cisco-Guid: 1427005942-4142731737-2150891535-615471488\r\n"
            "User-Agent: Cisco-SIPGateway/IOS-12.x\r\n"
            "Allow: INVITE, OPTIONS, BYE, CANCEL, ACK, PRACK, COMET, REFER, SUBSCRIBE, NOTIFY, INFO, UPDATE, REGISTER\r\n"
            "Cseq: 101 INVITE\r\n"
            "Max-Forwards: 9\r\n"
            "Timestamp: 1121709917\r\n"
            "Contact: <sip:10.21.128.204:5060>\r\n"
            "Expires: 180\r\n"
            "Allow-Events: telephone-event\r\n"
            "Mime-Version: 1.0\r\n"
            "Content-Type: multipart/mixed;boundary=uniqueBoundary\r\n"
            "Content-Length: 561\r\n"
            "\r\n"
            "--uniqueBoundary\r\n"
            "Content-Type: application/sdp\r\n"
            "\r\n"
            "v=0\r\n"
            "o=CiscoSystemsSIP-GW-UserAgent 9773 1231 IN IP4 10.21.128.204\r\n"
            "s=SIP Call\r\n"
            "c=IN IP4 10.21.128.204\r\n"
            "t=0 0\r\n"
            "m=audio 16634 RTP/AVP 0 98\r\n"
            "c=IN IP4 10.21.128.204\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
            "a=rtpmap:98 telephone-event/8000\r\n"
            "a=fmtp:98 0-16\r\n"
            "a=ptime:20\r\n"
            "--uniqueBoundary\r\n"
            "Content-Type: application/gtd\r\n"
            "Content-Disposition: signal;handling=optional\r\n"
            "\r\n"
            "IAM,\r\n"
            "PRN,isdn*,,NT100,\r\n"
            "USI,rate,c,s,c,1\r\n"
            "USI,lay1,ulaw\r\n"
            "TMR,00\r\n"
            "CPN,04,,1,65681\r\n"
            "CPC,09\r\n"
            "FCI,,,,,,,y,\r\n"
            "GCI,550e61f6f6ed11d98034000f24af5980\r\n"
            "\r\n"
            "--uniqueBoundary--\r\n"
            ;

         const char* correctBody =
            "v=0\r\n"
            "o=CiscoSystemsSIP-GW-UserAgent 9773 1231 IN IP4 10.21.128.204\r\n"
            "s=SIP Call\r\n"
            "c=IN IP4 10.21.128.204\r\n"
            "t=0 0\r\n"
            "m=audio 16634 RTP/AVP 0 98\r\n"
            "c=IN IP4 10.21.128.204\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
            "a=rtpmap:98 telephone-event/8000\r\n"
            "a=fmtp:98 0-16\r\n"
            "a=ptime:20\r\n"
            ;

         SipMessage testMsg( MultipartBodyMessage, strlen( MultipartBodyMessage ) );

         const SdpBody* sdpBody = testMsg.getSdpBody();
	 KNOWN_BUG("No support for multipart SDP", "XX-9198");
         CPPUNIT_ASSERT(sdpBody);

         UtlString theBody;
         ssize_t theLength;

         sdpBody->getBytes(&theBody, &theLength);
         sdpBody->findMediaType("audio", 0);

         ASSERT_STR_EQUAL(correctBody,theBody.data());
         delete sdpBody;
      };


   void testGetViaFieldSubField()
      {
         // Test that the getViaFieldSubField method returns the right results,
         // especially when Via values are combined in one header.

         const char* message1 =
            "SIP/2.0 481 Call Leg/Transaction Does Not Exist\r\n"
            "Via: SIP/2.0/UDP 10.0.11.35:5080;branch=z9hG4bK-80e0607bee4944e9ecb678caae8638d5;received=10.0.11.37,"
            "SIP/2.0/UDP 10.0.11.35;branch=z9hG4bK-379fceb40dc3c5716a3f167d93ceadf4;received=10.0.11.37,"
            "SIP/2.0/UDP 10.0.11.35:5080;branch=z9hG4bK-d05de917f970cd88ea048891ea57f140;received=10.0.11.37,"
            "SIP/2.0/UDP 10.0.11.35;branch=z9hG4bK-09d4d158ad31b82192efa4795b49df90;received=10.0.11.37,"
            "SIP/2.0/UDP 10.0.8.90:5060;branch=z9hG4bK5fa09267\r\n"
            "From: \"joanne brunet\" <sip:245@jaguar.local>;tag=0002fd3bb5770ab64fcc4d65-34791f85\r\n"
            "To: <sip:*4706@jaguar.local>\r\n"
            "Call-ID: 0002fd3b-b5770020-6a00fe74-11feac1a@10.0.8.90\r\n"
            "Date: Wed, 19 Apr 2006 13:19:40 GMT\r\n"
            "CSeq: 101 INVITE\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         // Same as message1, but with each Via value in a separate header.
         const char* message2 =
            "SIP/2.0 481 Call Leg/Transaction Does Not Exist\r\n"
            "Via: SIP/2.0/UDP 10.0.11.35:5080;branch=z9hG4bK-80e0607bee4944e9ecb678caae8638d5;received=10.0.11.37\r\n"
            "Via: SIP/2.0/UDP 10.0.11.35;branch=z9hG4bK-379fceb40dc3c5716a3f167d93ceadf4;received=10.0.11.37\r\n"
            "Via: SIP/2.0/UDP 10.0.11.35:5080;branch=z9hG4bK-d05de917f970cd88ea048891ea57f140;received=10.0.11.37\r\n"
            "Via: SIP/2.0/UDP 10.0.11.35;branch=z9hG4bK-09d4d158ad31b82192efa4795b49df90;received=10.0.11.37\r\n"
            "Via: SIP/2.0/UDP 10.0.8.90:5060;branch=z9hG4bK5fa09267\r\n"
            "From: \"joanne brunet\" <sip:245@jaguar.local>;tag=0002fd3bb5770ab64fcc4d65-34791f85\r\n"
            "To: <sip:*4706@jaguar.local>\r\n"
            "Call-ID: 0002fd3b-b5770020-6a00fe74-11feac1a@10.0.8.90\r\n"
            "Date: Wed, 19 Apr 2006 13:19:40 GMT\r\n"
            "CSeq: 101 INVITE\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         // The Via values.
         const char* (vias[]) = {
            "SIP/2.0/UDP 10.0.11.35:5080;branch=z9hG4bK-80e0607bee4944e9ecb678caae8638d5;received=10.0.11.37",
            "SIP/2.0/UDP 10.0.11.35;branch=z9hG4bK-379fceb40dc3c5716a3f167d93ceadf4;received=10.0.11.37",
            "SIP/2.0/UDP 10.0.11.35:5080;branch=z9hG4bK-d05de917f970cd88ea048891ea57f140;received=10.0.11.37",
            "SIP/2.0/UDP 10.0.11.35;branch=z9hG4bK-09d4d158ad31b82192efa4795b49df90;received=10.0.11.37",
            "SIP/2.0/UDP 10.0.8.90:5060;branch=z9hG4bK5fa09267",
         };

         UtlString value;
         size_t i;

         SipMessage testMessage1(message1, strlen(message1));

         for (i = 0;
              i < sizeof (vias) / sizeof (vias[0]) &&
                 testMessage1.getViaFieldSubField(&value, i);
              i++)
         {
            char buffer[100];
            sprintf(buffer,
                    "testMessage1.getViaFieldSubField(..., %zu) == vias[%zu]",
                    i, i);
            ASSERT_STR_EQUAL_MESSAGE(buffer, vias[i], value.data());
         }
         CPPUNIT_ASSERT_EQUAL(i, (sizeof (vias) / sizeof (vias[0])));

         SipMessage testMessage2(message2, strlen(message2));

         for (i = 0;
              i < sizeof (vias) / sizeof (vias[0]) &&
                 testMessage2.getViaFieldSubField(&value, i);
              i++)
         {
            char buffer[100];
            sprintf(buffer,
                    "testMessage2.getViaFieldSubField(..., %zu) == vias[%zu]",
                    i, i);
            ASSERT_STR_EQUAL_MESSAGE(buffer, vias[i], value.data());
         }
         CPPUNIT_ASSERT_EQUAL((char*)i, (char*)(sizeof (vias) / sizeof (vias[0])));
      };


   void testGetAllowEventField()
      {
         UtlString allowEventField;
         UtlString package;
         UtlString id;
         UtlHashMap params;

         const char* SubscribeMessage =
            "SUBSCRIBE sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Max-Forwards: 20\r\n"
            "Allow-Events: abc\r\n"
            "Allow-Events: def\r\n"
            "Allow-Events: \r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Event: the-package\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg( SubscribeMessage, strlen( SubscribeMessage ) );

         CPPUNIT_ASSERT(testMsg.getAllowEventsField(allowEventField));
         ASSERT_STR_EQUAL("abc, def", allowEventField.data());
      };

   void testSetAllowEventField()
   {
      const char* SimpleMessage =
         "REGISTER sip:sipx.local SIP/2.0\r\n"
         "Via: SIP/2.0/UDP sipx.local2:3355;branch=z9hG4bK-10cb6f93ea3d;test=2\r\n"
         "To: sip:sipx.local\r\n"
         "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 REGISTER\r\n"
         "User-Agent: sipsend/0.01\r\n"
         "Contact: me@127.0.0.1\r\n"
         "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

      testMsg.setAllowEventsField( "abc" );

      const char* ReferenceMessage =
         "REGISTER sip:sipx.local SIP/2.0\r\n"
         "Via: SIP/2.0/UDP sipx.local2:3355;branch=z9hG4bK-10cb6f93ea3d;test=2\r\n"
         "To: sip:sipx.local\r\n"
         "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 REGISTER\r\n"
         "User-Agent: sipsend/0.01\r\n"
         "Contact: me@127.0.0.1\r\n"
         "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
         "Content-Length: 0\r\n"
         "Allow-Events: abc\r\n"
         "\r\n";
      SipMessage refMsg( ReferenceMessage, strlen( ReferenceMessage ) );

      UtlString modifiedMsgString, referenceMsgString;
      ssize_t msgLen;
      testMsg.getBytes(&modifiedMsgString, &msgLen);
      refMsg.getBytes(&referenceMsgString, &msgLen);

      ASSERT_STR_EQUAL(referenceMsgString.data(), modifiedMsgString.data());
   }

   void testGetEventField()
      {
         UtlString fullEventField;
         UtlString package;
         UtlString id;
         UtlHashMap params;

         const char* SubscribeMessage =
            "SUBSCRIBE sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Event: the-package\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg( SubscribeMessage, strlen( SubscribeMessage ) );

         // use the raw interface to get the full field value
         CPPUNIT_ASSERT(testMsg.getEventField(fullEventField));
         ASSERT_STR_EQUAL("the-package",fullEventField.data());

         // use the parsing interface, but don't ask for the parameters
         CPPUNIT_ASSERT(testMsg.getEventFieldParts(&package));
         ASSERT_STR_EQUAL("the-package",package.data());

         // use the parsing interface and get the parameters (which should be empty)
         CPPUNIT_ASSERT(testMsg.getEventFieldParts(&package, &id, &params));
         ASSERT_STR_EQUAL("the-package",package.data());
         CPPUNIT_ASSERT(id.isNull());
         CPPUNIT_ASSERT(params.isEmpty());
      };

   void testGetEventFieldId()
      {
         UtlString fullEventField;
         UtlString package;
         UtlString id;
         UtlHashMap params;

         const char* SubscribeMessageWithId =
            "SUBSCRIBE sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Event: the-package;id=45wwrt2\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsgWithId( SubscribeMessageWithId, strlen( SubscribeMessageWithId ) );

         // use the raw interface to get the full field value
         CPPUNIT_ASSERT(testMsgWithId.getEventField(fullEventField));
         ASSERT_STR_EQUAL("the-package;id=45wwrt2",fullEventField.data());

         // use the parsing interface, but don't ask for the parameters
         CPPUNIT_ASSERT(testMsgWithId.getEventFieldParts(&package));
         ASSERT_STR_EQUAL("the-package",package.data());

         // use the parsing interface and get the parameters (which should be empty)
         CPPUNIT_ASSERT(testMsgWithId.getEventFieldParts(&package, &id, &params));
         ASSERT_STR_EQUAL("the-package",package.data());
         ASSERT_STR_EQUAL("45wwrt2",id.data());
         CPPUNIT_ASSERT(params.isEmpty());
      };

      void testGetEventFieldWithIdSpace()
      {
         UtlString fullEventField;
         UtlString package;
         UtlString id;
         UtlHashMap params;

         const char* SubscribeMessageWithIdSpace =
            "SUBSCRIBE sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Event: the-package; id=45wwrt2\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsgWithIdSpace( SubscribeMessageWithIdSpace, strlen( SubscribeMessageWithIdSpace ) );

         // use the raw interface to get the full field value
         CPPUNIT_ASSERT(testMsgWithIdSpace.getEventField(fullEventField));
         ASSERT_STR_EQUAL("the-package; id=45wwrt2",fullEventField.data());

         // use the parsing interface, but don't ask for the parameters
         CPPUNIT_ASSERT(testMsgWithIdSpace.getEventFieldParts(&package));
         ASSERT_STR_EQUAL("the-package",package.data());

         // use the parsing interface and get the parameters (which should be empty)
         CPPUNIT_ASSERT(testMsgWithIdSpace.getEventFieldParts(&package, &id, &params));
         ASSERT_STR_EQUAL("the-package",package.data());
         ASSERT_STR_EQUAL("45wwrt2",id.data());
         CPPUNIT_ASSERT(params.isEmpty());
      };

      void testGetEventFieldWithEqualSpace()
      {
         UtlString fullEventField;
         UtlString package;
         UtlString id;
         UtlHashMap params;

         const char* SubscribeMessageWithEqualSpace =
            "SUBSCRIBE sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag = 30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Event: the-package; id = 45wwrt2\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsgWithEqualSpace( SubscribeMessageWithEqualSpace, strlen( SubscribeMessageWithEqualSpace ) );

         // use the raw interface to get the full field value
         CPPUNIT_ASSERT(testMsgWithEqualSpace.getEventField(fullEventField));
         ASSERT_STR_EQUAL("the-package; id = 45wwrt2",fullEventField.data());

         // use the parsing interface, but don't ask for the parameters
         CPPUNIT_ASSERT(testMsgWithEqualSpace.getEventFieldParts(&package));
         ASSERT_STR_EQUAL("the-package",package.data());

         // use the parsing interface and get the parameters (which should be empty)
         CPPUNIT_ASSERT(testMsgWithEqualSpace.getEventFieldParts(&package, &id, &params));
         ASSERT_STR_EQUAL("the-package",package.data());
         ASSERT_STR_EQUAL("45wwrt2",id.data());
         CPPUNIT_ASSERT(params.isEmpty());
      };

      void testGetEventFieldWithTrailingSpace()
      {
         UtlString fullEventField;
         UtlString package;
         UtlString id;
         UtlHashMap params;

         const char* SubscribeMessageWithTrailingSpace =
            "SUBSCRIBE sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>;tag=30543f3483e1cb11ecb40866edd3295b ;foo=bar\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Event: the-package;id=45wwrt2 ;foo=bar\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsgWithTrailingSpace( SubscribeMessageWithTrailingSpace, strlen( SubscribeMessageWithTrailingSpace ) );

         // use the raw interface to get the full field value
         CPPUNIT_ASSERT(testMsgWithTrailingSpace.getEventField(fullEventField));
         ASSERT_STR_EQUAL("the-package;id=45wwrt2 ;foo=bar",fullEventField.data());

         // use the parsing interface, but don't ask for the parameters
         CPPUNIT_ASSERT(testMsgWithTrailingSpace.getEventFieldParts(&package));
         ASSERT_STR_EQUAL("the-package",package.data());

         // use the parsing interface and get the parameters (which should be empty)
         CPPUNIT_ASSERT(testMsgWithTrailingSpace.getEventFieldParts(&package, &id, &params));
         ASSERT_STR_EQUAL("the-package",package.data());
         ASSERT_STR_EQUAL("45wwrt2",id.data());
         // There is one other parameter in this test, so !params.isEmpty().
         CPPUNIT_ASSERT(!params.isEmpty());
      };

      void testGetEventFieldWithParams()
      {
         UtlString fullEventField;
         UtlString package;
         UtlString id;
         UtlHashMap params;
         UtlString* paramValue;

         const char* SubscribeMessageWithParams =
            "SUBSCRIBE sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Event: the-package;p1=one;id=45wwrt2;p2=two\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsgWithParams( SubscribeMessageWithParams, strlen( SubscribeMessageWithParams ) );

         // use the raw interface to get the full field value
         CPPUNIT_ASSERT(testMsgWithParams.getEventField(fullEventField));
         ASSERT_STR_EQUAL("the-package;p1=one;id=45wwrt2;p2=two",fullEventField.data());

         // use the parsing interface, but don't ask for the parameters
         CPPUNIT_ASSERT(testMsgWithParams.getEventFieldParts(&package));
         ASSERT_STR_EQUAL("the-package",package.data());

         // use the parsing interface and get the parameters (which should have values)
         CPPUNIT_ASSERT(testMsgWithParams.getEventFieldParts(&package, &id, &params));
         ASSERT_STR_EQUAL("the-package",package.data());
         ASSERT_STR_EQUAL("45wwrt2",id.data());
         CPPUNIT_ASSERT(params.entries()==2);

         UtlString paramName1("p1");
         CPPUNIT_ASSERT(NULL != (paramValue = dynamic_cast<UtlString*>(params.findValue(&paramName1))));
         ASSERT_STR_EQUAL("one",paramValue->data());

         UtlString paramName2("p2");
         CPPUNIT_ASSERT(NULL != (paramValue = dynamic_cast<UtlString*>(params.findValue(&paramName2))));
         ASSERT_STR_EQUAL("two",paramValue->data());
      }

      void testGetEventFieldWithParamButNoValue()
      {
         UtlString fullEventField;
         UtlString package;
         UtlString id;
         UtlHashMap params;
         UtlString* paramValue;

         const char* SubscribeMessageWithParams =
            "SUBSCRIBE sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Event: the-package;sla\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsgWithParams( SubscribeMessageWithParams, strlen( SubscribeMessageWithParams ) );

         // use the raw interface to get the full field value
         CPPUNIT_ASSERT(testMsgWithParams.getEventField(fullEventField));
         ASSERT_STR_EQUAL("the-package;sla",fullEventField.data());

         // use the parsing interface, but don't ask for the parameters
         CPPUNIT_ASSERT(testMsgWithParams.getEventFieldParts(&package));
         ASSERT_STR_EQUAL("the-package",package.data());

         // use the parsing interface and get the parameters (which might not have values)
         CPPUNIT_ASSERT(testMsgWithParams.getEventFieldParts(&package, &id, &params));
         ASSERT_STR_EQUAL("the-package",package.data());
         CPPUNIT_ASSERT(id.isNull());
         CPPUNIT_ASSERT(params.entries()==1);

         UtlString paramName1("sla");
         CPPUNIT_ASSERT(NULL != (paramValue = dynamic_cast<UtlString*>(params.findValue(&paramName1))));
         ASSERT_STR_EQUAL("",paramValue->data());
      }

   void testGetToAddress()
      {
         struct test {
	   const char* string;	// Input string.
	   int port;		// Expected returned to-address port.
         };

	 struct test tests[] = {
	   { "sip:foo@bar", PORT_NONE },
	   { "sip:foo@bar:5060", 5060 },
	   { "sip:foo@bar:1", 1 },
	   { "sip:foo@bar:100", 100 },
	   { "sip:foo@bar:65535", 65535 },
         };

         UtlString address;
         int port;
         UtlString protocol;
         UtlString user;
         UtlString userLabel;
         UtlString tag;
         // Buffer to compose message.
         char message[1000];
         // Message template into which to insert the To address.
         const char* message_template =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "To: %s\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         for (unsigned int i = 0; i < sizeof (tests) / sizeof (tests[0]);
              i++)
         {
            // Compose the message.
            sprintf(message, message_template, tests[i].string);
            SipMessage sipMessage(message, strlen(message));

            sipMessage.getToAddress(&address, &port, &protocol, &user,
                                    &userLabel, &tag);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(tests[i].string, tests[i].port, port);
         }
      }

   void testGetFromAddress()
      {
         struct test {
	   const char* string;	// Input string.
	   int port;		// Expected returned from-address port.
         };

	 struct test tests[] = {
	   { "sip:foo@bar", PORT_NONE },
	   { "sip:foo@bar:5060", 5060 },
	   { "sip:foo@bar:1", 1 },
	   { "sip:foo@bar:100", 100 },
	   { "sip:foo@bar:65535", 65535 },
         };

         UtlString address;
         int port;
         UtlString protocol;
         UtlString user;
         UtlString userLabel;
         UtlString tag;
         // Buffer to compose message.
         char message[1000];
         // Message template into which to insert the From address.
         const char* message_template =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "To: Sip Send <sip:sipsend@pingtel.org>\r\n"
            "From: %s; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         for (unsigned int i = 0; i < sizeof (tests) / sizeof (tests[0]);
              i++)
         {
            // Compose the message.
            sprintf(message, message_template, tests[i].string);
            SipMessage sipMessage(message, strlen(message));

            sipMessage.getFromAddress(&address, &port, &protocol, &user,
                                    &userLabel, &tag);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(tests[i].string, tests[i].port, port);
         }
      }

   void testGetResponseSendAddress()
      {
         // Message templates into which to insert the address.

         // Template has 2 Via's, to make sure the function is looking
         // at the right Via.
         const char* message_template2 =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP %s;branch=z9hG4bK-foobarbazquux\r\n"
            "Via: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "\r\n";

         // Template has 1 Via.
         const char* message_template1 =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP %s;branch=z9hG4bK-foobarbazquux\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "\r\n";

         // Template has 0 Via's.
         // The From address is used.
         const char* message_template0 =
            "REGISTER sip:sipsend@pingtel.org SIP/2.0\r\n"
            "To: sip:sipx.local\r\n"
            "From: %s; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "\r\n";

         struct test {
            const char* message_template;
            const char* address;
            int port;
         };

         struct test tests[] = {
            { message_template0, "sip:foo@bar", PORT_NONE },
            { message_template0, "sip:foo@bar:0", 0 },
            { message_template0, "sip:foo@bar:100", 100 },
            { message_template0, "sip:foo@bar:5060", 5060 },
            { message_template0, "sip:foo@bar:65535", 65535 },
            { message_template1, "sip:foo@bar", PORT_NONE },
            { message_template1, "sip:foo@bar:0", 0 },
            { message_template1, "sip:foo@bar:100", 100 },
            { message_template1, "sip:foo@bar:5060", 5060 },
            { message_template1, "sip:foo@bar:65535", 65535 },
            { message_template2, "sip:foo@bar", PORT_NONE },
            { message_template2, "sip:foo@bar:0", 0 },
            { message_template2, "sip:foo@bar:100", 100 },
            { message_template2, "sip:foo@bar:5060", 5060 },
            { message_template2, "sip:foo@bar:65535", 65535 },
         };

         // Buffer to compose message.
         char message[1000];

         UtlString address;
         int port;
         UtlString protocol;

         for (unsigned int i = 0; i < sizeof (tests) / sizeof (tests[0]);
              i++)
         {
            // Compose the message.
            sprintf(message, tests[i].message_template, tests[i].address);
            SipMessage sipMessage(message, strlen(message));

            sipMessage.getResponseSendAddress(address,
                                              port,
                                              protocol);
            char number[10];
            sprintf(number, "Test %d", i);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(number, tests[i].port, port);
         }
      }

   void testParseAddressFromUriPort()
      {
         struct test {
	   const char* string;	// Input string.
	   int port;		// Expected returned port.
         };

	 struct test tests[] = {
	   { "sip:foo@bar", PORT_NONE },
	   { "sip:foo@bar:5060", 5060 },
	   { "sip:foo@bar:1", 1 },
	   { "sip:foo@bar:100", 100 },
	   { "sip:foo@bar:65535", 65535 },
         };

         UtlString address;
         int port;
         UtlString protocol;
         UtlString user;
         UtlString userLabel;
         UtlString tag;

         for (unsigned int i = 0; i < sizeof (tests) / sizeof (tests[0]);
              i++)
         {
            SipMessage::parseAddressFromUri(tests[i].string,
                                            &address, &port, &protocol, &user,
                                            &userLabel, &tag);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(tests[i].string, tests[i].port, port);
         }
      }

   void testProbPort()
      {
         const char *szUrl = "\"Display@Name\"<sip:username@sipserver;transport=tcp>;tag=1234-2345";

         UtlString address;
         int       port;
         UtlString protocol;
         UtlString user;
         UtlString userLabel;
         UtlString tag;

         SipMessage::parseAddressFromUri(szUrl,
                                         &address,
                                         &port,
                                         &protocol,
                                         &user,
                                         &userLabel,
                                         &tag);

         CPPUNIT_ASSERT_EQUAL(PORT_NONE, port);

         ASSERT_STR_EQUAL("username", user);
         ASSERT_STR_EQUAL("\"Display@Name\"", userLabel);
         ASSERT_STR_EQUAL("1234-2345", tag);
         ASSERT_STR_EQUAL("sipserver", address);

         OsSocket::IpProtocolSocketType protoNumber;
         SipMessage::convertProtocolStringToEnum(protocol.data(), protoNumber);

         CPPUNIT_ASSERT_EQUAL(OsSocket::TCP, protoNumber);
      }

   void testCodecError()
      {
         const char* message =
            "INVITE sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         // Construct a message.
         SipMessage testMsg(message, strlen(message));

         // Construct a SipUserAgent to provide the "agent name" for
         // the Warning header.
         SipUserAgent user_agent(0,
                                 0,
                                 -1,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 SIP_DEFAULT_RTT,
                                 TRUE,
                                 -1,
                                 OsServerTask::DEF_MAX_MSGS);

         // Construct error response for insufficient codeecs.
         SipMessage response;
         response.setInviteBadCodecs(&testMsg, &user_agent);

         // Check that the response code is 488.
         // Note this code is hard-coded here, because it is fixed by the
         // standard.  Using the #define would leave this test vulnerable
         // to mistakes in the #define!
         CPPUNIT_ASSERT_EQUAL(488, response.getResponseStatusCode());

         // Check that the Warning header is correct.
         const char* warning_header = response.getHeaderValue(0, "Warning");
         CPPUNIT_ASSERT(warning_header != NULL);

         // Parse the Warning header value.
         int code;
         char agent[128], text[128];
         int r =
            sscanf(warning_header, " %d %s \"%[^\"]\"", &code, agent, text);
         // Ensure that all three fields could be found.
         CPPUNIT_ASSERT_EQUAL(3, r);
         // Check the warning code.
         // Note this code is hard-coded here, because it is fixed by the
         // standard.  Using the #define would leave this test vulnerable
         // to mistakes in the #define!
         CPPUNIT_ASSERT_EQUAL(305, code);
         // Figure out what the agent value should be.
         UtlString address;
         int port;
         user_agent.getViaInfo(OsSocket::UDP, address, port);
         char agent_expected[128];
         strcpy(agent_expected, address.data());
         if (port != 5060)      // PORT_NONE
         {
            sprintf(&agent_expected[strlen(agent_expected)], ":%d", port);
         }
         // Check the agent value.
         ASSERT_STR_EQUAL(agent_expected, agent);
      }

 void testSdpParse()
   {
        const char* sip = "INVITE 14 SIP/2.0\nContent-Type:application/sdp\n\n"
            "v=0\nm=audio 49170 RTP/AVP 0\nc=IN IP4 224.2.17.12/127";

        SipMessage *msg = new SipMessage(sip);
        const SdpBody *sdp = msg->getSdpBody();

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

        SipMessage* msgCopy = new SipMessage(*msg);
        CPPUNIT_ASSERT_MESSAGE("NULL message copy", msgCopy != NULL);
        const SdpBody *sdpCopy = msgCopy->getSdpBody();
        CPPUNIT_ASSERT_MESSAGE("NULL SDP copy", sdpCopy != NULL);
        const char* sdpCopyBytes = NULL;
        ssize_t sdpCopyLen = 0;
        sdpCopy->getBytes(&sdpCopyBytes, &sdpCopyLen);
        //printf("SDP copy length: %d\n%s\n", sdpCopyLen, sdpCopyBytes);
        CPPUNIT_ASSERT_MESSAGE("Null sdp copy serialized content", sdpCopyBytes != NULL);
        CPPUNIT_ASSERT_MESSAGE("SDP does not match expected content",
            strcmp(referenceSdp, sdpCopyBytes) == 0);
        delete sdp;
        delete sdpCopy;
   }

#define NON_SDP_REFERENCE_CONTENT "<FOOSTUFF>\n   <BAR/>\n\r</FOOSTUFF>\n"

   void testNonSdpSipMessage()
   {
        const char* referenceContent = NON_SDP_REFERENCE_CONTENT;
        const char* sip = "INVITE 14 SIP/2.0\nContent-Type:application/fooStuff\n\n"
            NON_SDP_REFERENCE_CONTENT;

        SipMessage *msg = new SipMessage(sip);
        const SdpBody *sdp = msg->getSdpBody();

        CPPUNIT_ASSERT_MESSAGE("sdp body not expected", sdp == NULL);

        const HttpBody* fooBody = msg->getBody();

        const char* fooBytes = NULL;
        ssize_t fooByteLength = 0;
        fooBody->getBytes(&fooBytes, &fooByteLength);
        for(ssize_t iii = 0; iii < fooByteLength; iii++)
        {
            if(referenceContent[iii] != fooBytes[iii])
            {
                printf("index[%zd]: expected: %d got: %d\n",
                    iii, referenceContent[iii], fooBytes[iii]);
            }
        }
        CPPUNIT_ASSERT_MESSAGE("Null foo serialized content", fooBytes != NULL);
        CPPUNIT_ASSERT_MESSAGE("serialized content does not match expected content",
            strcmp(referenceContent, fooBytes) == 0);

        SipMessage* msgCopy = new SipMessage(*msg);
        CPPUNIT_ASSERT_MESSAGE("NULL message copy", msgCopy != NULL);
        const HttpBody *fooCopy = msgCopy->getBody();
        CPPUNIT_ASSERT_MESSAGE("NULL foo body copy", fooCopy != NULL);
        const char* fooCopyBytes = NULL;
        ssize_t fooCopyLen = 0;
        fooCopy->getBytes(&fooCopyBytes, &fooCopyLen);
        //printf("foo copy length: %d\n%s\n", fooCopyLen, fooCopyBytes);
        CPPUNIT_ASSERT_MESSAGE("Null foo copy serialized content", fooCopyBytes != NULL);
        CPPUNIT_ASSERT_MESSAGE("foo body copy does not match expected content",
            strcmp(referenceContent, fooCopyBytes) == 0);
   }

   void testSetInviteDataHeaders()
   {
      // Test that SipMessage::setInviteData applies headers in the
      // To: URI correctly to the SIP message body.

      // List of headers that should be settable from the URI.
      const char* settable_headers[] =
         {
            SIP_SUBJECT_FIELD,
            SIP_ACCEPT_LANGUAGE_FIELD,
            "Alert-Info",
            "Call-Info",
            SIP_WARNING_FIELD,
            "Error-Info",
         };

      // For each field.
      for (unsigned int i = 0; i < sizeof (settable_headers) / sizeof (settable_headers[0]); i++)
      {
         // The name of the header.
         const char* header_name = settable_headers[i];

         // Create an empty SIP message.
         SipMessage *msg = new SipMessage();

         // Create a To URI containing the header.
         char to_URI[100];
         sprintf(to_URI, "<sip:to@example.com?%s=value1>", header_name);

         // Create the SIP message.
         // Since numRtpcodecs = 0, none of the RTP fields are used to produce SDP.
         msg->setInviteData("sip:from@example.com", // fromField
                            to_URI, // toField,
                            "sip:remotecontact@example.com", // farEndContact
                            "sip:contact@example.com", // contactUrl
                            "callid@example.com", // callId
                            NULL, // rtpAddress
                            0, // rtpAudioPort
                            0, // rtcpAudioPort
                            0, // rtpVideoPort
                            0, // rtcpVideoPort
                            NULL, // srtpParams
                            0, // sequenceNumber
                            0, // numRtpCodecs
                            NULL, // rtpCodecs
                            17 // sessionReinviteTimer
            );

#if 0
         UtlString p;
         ssize_t l;
         msg->getBytes(&p, &l);
         fprintf(stderr,
                 "testSetInviteDataHeaders for %s after first setInviteData:\n%s\n",
                 header_name, p.data());
#endif

         const char* v = msg->getHeaderValue(0, header_name);
         CPPUNIT_ASSERT_MESSAGE(header_name,
                                v != NULL && strcmp(v, "value1") == 0);

         // Create a second To URI containing the header.
         sprintf(to_URI, "<sip:to@example.com?%s=value2>", header_name);

         // Update the SIP message, creating a second value for the header.
         // Since numRtpcodecs = 0, none of the RTP fields are used to produce SDP.
         msg->setInviteData("sip:from@example.com", // fromField
                            to_URI, // toField,
                            "sip:remotecontact@example.com", // farEndContact
                            "sip:contact@example.com", // contactUrl
                            "callid@example.com", // callId
                            NULL, // rtpAddress
                            0, // rtpAudioPort
                            0, // rtcpAudioPort
                            0, // rtpVideoPort
                            0, // rtcpVideoPort
                            NULL, // srtpParams
                            0, // sequenceNumber
                            0, // numRtpCodecs
                            NULL, // rtpCodecs
                            17 // sessionReinviteTimer
            );

#if 0
         msg->getBytes(&p, &l);
         fprintf(stderr,
                 "testSetInviteDataHeaders for %s after second setInviteData:\n%s\n",
                 header_name, p.data());
#endif

         v = msg->getHeaderValue(0, header_name);
         CPPUNIT_ASSERT_MESSAGE(header_name,
                                v != NULL && strcmp(v, "value1") == 0);

         v = msg->getHeaderValue(1, header_name);
         CPPUNIT_ASSERT_MESSAGE(header_name,
                                v != NULL && strcmp(v, "value2") == 0);

         delete msg;
      }
   }

   void testSetInviteDataHeadersUnique()
   {
      // Test that SipMessage::setInviteData applies headers in the
      // To: URI correctly to the SIP message body.

      // List of headers that should be settable from the URI, but are
      // allowed only one value, so the URI overrides what is already in the
      // message.
      const char* settable_unique_headers[] =
         {
            SIP_EXPIRES_FIELD,
         };

      // For each field.
      for (unsigned int i = 0; i < sizeof (settable_unique_headers) / sizeof (settable_unique_headers[0]); i++)
      {
         // The name of the header.
         const char* header_name = settable_unique_headers[i];

         // Create an empty SIP message.
         SipMessage *msg = new SipMessage();

         // Create a To URI containing the header.
         char to_URI[100];
         sprintf(to_URI, "<sip:to@example.com?%s=value1>", header_name);

         // Create the SIP message.
         // Since numRtpcodecs = 0, none of the RTP fields are used to produce SDP.
         msg->setInviteData("sip:from@example.com", // fromField
                            to_URI, // toField,
                            "sip:remotecontact@example.com", // farEndContact
                            "sip:contact@example.com", // contactUrl
                            "callid@example.com", // callId
                            NULL, // rtpAddress
                            0, // rtpAudioPort
                            0, // rtcpAudioPort
                            0, // rtpVideoPort
                            0, // rtcpVideoPort
                            NULL, // srtpParams
                            0, // sequenceNumber
                            0, // numRtpCodecs
                            NULL, // rtpCodecs
                            17 // sessionReinviteTimer
            );

#if 0
         UtlString p;
         ssize_t l;
         msg->getBytes(&p, &l);
         fprintf(stderr,
                 "testSetInviteDataHeaders for %s after first setInviteData:\n%s\n",
                 header_name, p.data());
#endif

         const char* v = msg->getHeaderValue(0, header_name);
         CPPUNIT_ASSERT_MESSAGE(header_name,
                                v != NULL && strcmp(v, "value1") == 0);

         // Create a second To URI containing the header.
         sprintf(to_URI, "<sip:to@example.com?%s=value2>", header_name);

         // Update the SIP message, creating a second value for the header.
         // Since numRtpcodecs = 0, none of the RTP fields are used to produce SDP.
         msg->setInviteData("sip:from@example.com", // fromField
                            to_URI, // toField,
                            "sip:remotecontact@example.com", // farEndContact
                            "sip:contact@example.com", // contactUrl
                            "callid@example.com", // callId
                            NULL, // rtpAddress
                            0, // rtpAudioPort
                            0, // rtcpAudioPort
                            0, // rtpVideoPort
                            0, // rtcpVideoPort
                            NULL, // srtpParams
                            0, // sequenceNumber
                            0, // numRtpCodecs
                            NULL, // rtpCodecs
                            17 // sessionReinviteTimer
            );

#if 0
         msg->getBytes(&p, &l);
         fprintf(stderr,
                 "testSetInviteDataHeaders for %s after second setInviteData:\n%s\n",
                 header_name, p.data());
#endif

         v = msg->getHeaderValue(0, header_name);
         CPPUNIT_ASSERT_MESSAGE(header_name,
                                v != NULL && strcmp(v, "value2") == 0);

         // Second value must not be present.
         v = msg->getHeaderValue(1, header_name);
         CPPUNIT_ASSERT_MESSAGE(header_name,
                                v == NULL ||strcmp(v, "") == 0);

         delete msg;
      }
   }

   void testSetInviteDataHeadersForbidden()
   {
      // Test that SipMessage::setInviteData applies headers in the
      // To: URI correctly to the SIP message body.

      // List of headers that should not be settable from the URI.
      const char* non_settable_headers[] =
         {
            SIP_CONTACT_FIELD,
            SIP_CALLID_FIELD,
            SIP_CSEQ_FIELD,
            SIP_VIA_FIELD,
            SIP_RECORD_ROUTE_FIELD
         };

      // For each field.
      for (unsigned int i = 0; i < sizeof (non_settable_headers) / sizeof (non_settable_headers[0]); i++)
      {
         // The name of the header.
         const char* header_name = non_settable_headers[i];

         // Create an empty SIP message.
         SipMessage *msg = new SipMessage();

         // Create a To URI containing the header.
         char to_URI[100];
         sprintf(to_URI, "<sip:to@example.com?%s=value1>", header_name);

         // Create the SIP message.
         // Since numRtpcodecs = 0, none of the RTP fields are used to produce SDP.
         msg->setInviteData("sip:from@example.com", // fromField
                            to_URI, // toField,
                            "sip:remotecontact@example.com", // farEndContact
                            "sip:contact@example.com", // contactUrl
                            "callid@example.com", // callId
                            NULL, // rtpAddress
                            0, // rtpAudioPort
                            0, // rtcpAudioPort
                            0, // rtpVideoPort
                            0, // rtcpVideoPort
                            NULL, // srtpParams
                            0, // sequenceNumber
                            0, // numRtpCodecs
                            NULL, // rtpCodecs
                            17 // sessionReinviteTimer
            );

#if 0
         UtlString p;
         ssize_t l;
         msg->getBytes(&p, &l);
         fprintf(stderr,
                 "testSetInviteDataHeaders for %s after first setInviteData:\n%s\n",
                 header_name, p.data());
#endif

         const char* v = msg->getHeaderValue(0, header_name);
         // Value must be absent, or NOT the specified value.
         CPPUNIT_ASSERT_MESSAGE(header_name,
                                v == NULL || strcmp(v, "value1") != 0);

         delete msg;
      }
   }

   void testApplyTargetUriHeaderParams()
      {
         UtlString dumpStr;
         ssize_t   dumpLength;

         dumpLength = 0;
         dumpLength = dumpLength; // suppress compiler warnings about unused variables

         {
            // add an arbitrary unknown header
            const char* rawmsg =
               "INVITE sip:sipx.local?arbitrary-header=foobar SIP/2.0\r\n"
               "To: sip:sipx.local\r\n"
               "From: Sip Send <sip:sender@example.org>; tag=ORIG-TAG\r\n"
               "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
               "Cseq: 1 INVITE\r\n"
               "Max-Forwards: 20\r\n"
               "Contact: me@127.0.0.1\r\n"
               "Content-Length: 0\r\n"
               "\r\n";

            SipMessage sipmsg(rawmsg, strlen(rawmsg));
            sipmsg.applyTargetUriHeaderParams();

            // sipmsg.getBytes(&dumpStr, &dumpLength);
            // printf( "\nMSG BEFORE:\n%s\nAFTER:\n%s\n", rawmsg, dumpStr.data());

            CPPUNIT_ASSERT_EQUAL(1, sipmsg.getCountHeaderFields("arbitrary-header"));
            ASSERT_STR_EQUAL("foobar", sipmsg.getHeaderValue(0, "arbitrary-header"));
            ASSERT_STR_EQUAL("INVITE sip:sipx.local SIP/2.0", sipmsg.getFirstHeaderLine());
         }

         {
            // add 2 of an arbitrary unknown header
            const char* rawmsg =
               "INVITE sip:sipx.local?arbitrary-header=foobar&arbitrary-header=again SIP/2.0\r\n"
               "To: sip:sipx.local\r\n"
               "From: Sip Send <sip:sender@example.org>; tag=ORIG-TAG\r\n"
               "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
               "Cseq: 1 INVITE\r\n"
               "Max-Forwards: 20\r\n"
               "Contact: me@127.0.0.1\r\n"
               "Content-Length: 0\r\n"
               "\r\n";

            SipMessage sipmsg(rawmsg, strlen(rawmsg));
            sipmsg.applyTargetUriHeaderParams();

            // sipmsg.getBytes(&dumpStr, &dumpLength);
            // printf( "\nMSG BEFORE:\n%s\nAFTER:\n%s\n", rawmsg, dumpStr.data());

            CPPUNIT_ASSERT_EQUAL(2, sipmsg.getCountHeaderFields("arbitrary-header"));
            ASSERT_STR_EQUAL("foobar", sipmsg.getHeaderValue(0, "arbitrary-header"));
            ASSERT_STR_EQUAL("again", sipmsg.getHeaderValue(1, "arbitrary-header"));
            ASSERT_STR_EQUAL("INVITE sip:sipx.local SIP/2.0", sipmsg.getFirstHeaderLine());
         }

         {
            // add an expires header
            const char* rawmsg =
               "INVITE sip:sipx.local?expires=10 SIP/2.0\r\n"
               "To: sip:sipx.local\r\n"
               "From: Sip Send <sip:sender@example.org>; tag=ORIG-TAG\r\n"
               "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
               "Cseq: 1 INVITE\r\n"
               "Max-Forwards: 20\r\n"
               "Contact: me@127.0.0.1\r\n"
               "Content-Length: 0\r\n"
               "\r\n";

            SipMessage sipmsg(rawmsg, strlen(rawmsg));
            sipmsg.applyTargetUriHeaderParams();

            // sipmsg.getBytes(&dumpStr, &dumpLength);
            // printf( "\nMSG BEFORE:\n%s\nAFTER:\n%s\n", rawmsg, dumpStr.data());

            CPPUNIT_ASSERT_EQUAL(1, sipmsg.getCountHeaderFields("expires"));
            ASSERT_STR_EQUAL("10", sipmsg.getHeaderValue(0, "expires"));
            ASSERT_STR_EQUAL("INVITE sip:sipx.local SIP/2.0", sipmsg.getFirstHeaderLine());
         }

         {
            // try to add 2 expires headers
            const char* rawmsg =
               "INVITE sip:sipx.local?expires=10&expires=20 SIP/2.0\r\n"
               "To: sip:sipx.local\r\n"
               "From: Sip Send <sip:sender@example.org>; tag=ORIG-TAG\r\n"
               "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
               "Cseq: 1 INVITE\r\n"
               "Max-Forwards: 20\r\n"
               "Contact: me@127.0.0.1\r\n"
               "Content-Length: 0\r\n"
               "\r\n";

            SipMessage sipmsg(rawmsg, strlen(rawmsg));
            sipmsg.applyTargetUriHeaderParams();

            // sipmsg.getBytes(&dumpStr, &dumpLength);
            // printf( "\nMSG BEFORE:\n%s\nAFTER:\n%s\n", rawmsg, dumpStr.data());

            CPPUNIT_ASSERT_EQUAL(1, sipmsg.getCountHeaderFields("expires"));
            ASSERT_STR_EQUAL("20", sipmsg.getHeaderValue(0, "expires"));
            ASSERT_STR_EQUAL("INVITE sip:sipx.local SIP/2.0", sipmsg.getFirstHeaderLine());
         }

#        if 0
         // these next two are broken because of un-escaping problems...
         {
            // try to add a From header with a display name
            const char* rawmsg =
               "INVITE sip:sipx.local?from=%22Foo+Bar%22+sip%3Afoo%40bar SIP/2.0\r\n"
               "To: sip:sipx.local\r\n"
               "From: Sip Send <sip:sender@example.org>; tag=ORIG-TAG\r\n"
               "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
               "Cseq: 1 INVITE\r\n"
               "Max-Forwards: 20\r\n"
               "Contact: me@127.0.0.1\r\n"
               "Content-Length: 0\r\n"
               "\r\n";

            SipMessage sipmsg(rawmsg, strlen(rawmsg));
            sipmsg.applyTargetUriHeaderParams();

            sipmsg.getBytes(&dumpStr, &dumpLength);
            printf( "\nMSG BEFORE:\n%s\nAFTER:\n%s\n", rawmsg, dumpStr.data());

            CPPUNIT_ASSERT_EQUAL(1, sipmsg.getCountHeaderFields("from"));
            ASSERT_STR_EQUAL("Foo Bar<sip:foo@bar>;tag=ORIG-TAG", sipmsg.getHeaderValue(0, "from"));

            CPPUNIT_ASSERT_EQUAL(1, sipmsg.getCountHeaderFields("x-original-from"));
            ASSERT_STR_EQUAL("From: Sip Send <sip:sender@example.org>; tag=ORIG-TAG",
                             sipmsg.getHeaderValue(0, "x-original-from"));

            ASSERT_STR_EQUAL("INVITE sip:sipx.local SIP/2.0", sipmsg.getFirstHeaderLine());
         }

         {
            // try to add a From header with a display name
            const char* rawmsg =
               "INVITE sip:sipx.local?from=Foo+Bar+%3Csip%3Afoo%40bar%3A SIP/2.0\r\n"
               "To: sip:sipx.local\r\n"
               "From: Sip Send <sip:sender@example.org>; tag=ORIG-TAG\r\n"
               "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
               "Cseq: 1 INVITE\r\n"
               "Max-Forwards: 20\r\n"
               "Contact: me@127.0.0.1\r\n"
               "Content-Length: 0\r\n"
               "\r\n";

            SipMessage sipmsg(rawmsg, strlen(rawmsg));
            sipmsg.applyTargetUriHeaderParams();

            sipmsg.getBytes(&dumpStr, &dumpLength);
            printf( "\nMSG BEFORE:\n%s\nAFTER:\n%s\n", rawmsg, dumpStr.data());

            CPPUNIT_ASSERT_EQUAL(1, sipmsg.getCountHeaderFields("from"));
            ASSERT_STR_EQUAL("Foo Bar<sip:foo@bar>;tag=ORIG-TAG", sipmsg.getHeaderValue(0, "from"));

            CPPUNIT_ASSERT_EQUAL(1, sipmsg.getCountHeaderFields("x-original-from"));
            ASSERT_STR_EQUAL("From: Sip Send <sip:sender@example.org>; tag=ORIG-TAG",
                             sipmsg.getHeaderValue(0, "x-original-from"));

            ASSERT_STR_EQUAL("INVITE sip:sipx.local SIP/2.0", sipmsg.getFirstHeaderLine());
         }
#        endif

         {
            // try to add a Route header
            const char* rawmsg =
               "INVITE sip:sipx.local?route=%3Csip%3Afoo%40bar%3Blr%3E SIP/2.0\r\n"
               "To: sip:sipx.local\r\n"
               "From: Sip Send <sip:sender@example.org>; tag=ORIG-TAG\r\n"
               "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
               "Cseq: 1 INVITE\r\n"
               "Max-Forwards: 20\r\n"
               "Contact: me@127.0.0.1\r\n"
               "Content-Length: 0\r\n"
               "\r\n";

            SipMessage sipmsg(rawmsg, strlen(rawmsg));
            sipmsg.applyTargetUriHeaderParams();

            // sipmsg.getBytes(&dumpStr, &dumpLength);
            // printf( "\nMSG BEFORE:\n%s\nAFTER:\n%s\n", rawmsg, dumpStr.data());

            CPPUNIT_ASSERT_EQUAL(1, sipmsg.getCountHeaderFields("route"));
            ASSERT_STR_EQUAL("<sip:foo@bar;lr>", sipmsg.getHeaderValue(0, "route"));
            ASSERT_STR_EQUAL("INVITE sip:sipx.local SIP/2.0", sipmsg.getFirstHeaderLine());
         }

         {
            // try to add a Route header when there is already one present
            const char* rawmsg =
               "INVITE sip:sipx.local?route=%3Csip%3Afoo%40bar%3Blr%3E SIP/2.0\r\n"
               "Route: <sip:original@route;lr>\r\n"
               "To: sip:sipx.local\r\n"
               "From: Sip Send <sip:sender@example.org>; tag=ORIG-TAG\r\n"
               "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
               "Cseq: 1 INVITE\r\n"
               "Max-Forwards: 20\r\n"
               "Contact: me@127.0.0.1\r\n"
               "Content-Length: 0\r\n"
               "\r\n";

            SipMessage sipmsg(rawmsg, strlen(rawmsg));
            sipmsg.applyTargetUriHeaderParams();

            // sipmsg.getBytes(&dumpStr, &dumpLength);
            // printf( "\nMSG BEFORE:\n%s\nAFTER:\n%s\n", rawmsg, dumpStr.data());

            CPPUNIT_ASSERT_EQUAL(1, sipmsg.getCountHeaderFields("route"));
            ASSERT_STR_EQUAL("<sip:foo@bar;lr>,<sip:original@route;lr>", sipmsg.getHeaderValue(0, "route"));
            ASSERT_STR_EQUAL("INVITE sip:sipx.local SIP/2.0", sipmsg.getFirstHeaderLine());
         }
      }

   void testCompactNames()
      {
         const char* CompactMessage =
            "METHOD sip:sipx.local SIP/2.0\r\n"
            "v: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "v: SIP/2.0/TCP sipx.remote:999999;branch=z9hG4bK-remote-tid\r\n"
            "t: sip:sipx.local\r\n"
            "f: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "i: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "o: event-package\r\n"
            "r: sip:refer@address.example.com\r\n"
            "b: sip:refered-by@address.example.com\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "s: Some silly subject\r\n"
            "k: a-supported-token\r\n"
            "m: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "l: 0\r\n"
            "c: application/sdp\r\n"
            "e: gzip\r\n"
            "\r\n";
         SipMessage testMsg( CompactMessage, strlen( CompactMessage ) );

         const char* LongForm =
            "METHOD sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "Via: SIP/2.0/TCP sipx.remote:999999;branch=z9hG4bK-remote-tid\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "Event: event-package\r\n"
            "Refer-To: sip:refer@address.example.com\r\n"
            "Referred-By: sip:refered-by@address.example.com\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Subject: Some silly subject\r\n"
            "Supported: a-supported-token\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Encoding: gzip\r\n"
            "\r\n";

         UtlString translated;
         ssize_t length;

         testMsg.getBytes(&translated, &length);

         ASSERT_STR_EQUAL(LongForm, translated.data());

      };

   void testGetContactUri()
      {
         // Template for messages.
         const char* message_skeleton =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: %s\r\n"
            "Expires: 300\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         // List of tests:  values for the Contact field and the URI
         // that should be extracted from them.
         struct test
         {
            const char* contact_field;
            const char* contact_uri;
         }
         tests[] =
         {
            { "sip:me@127.0.0.1", "sip:me@127.0.0.1" },
            { "<sip:me@127.0.0.1>", "sip:me@127.0.0.1" },
            { "sip:me@127.0.0.1;transport=udp", "sip:me@127.0.0.1" },
            { "<sip:me@127.0.0.1;transport=udp>", "sip:me@127.0.0.1;transport=udp" },
            { "<sip:me@127.0.0.1>;transport=udp", "sip:me@127.0.0.1" },
            { "\"Magic <trouble>\" <sip:me@127.0.0.1>", "sip:me@127.0.0.1" },
         };

         // Loop through all the cases.
         for (unsigned int i = 0; i < sizeof (tests) / sizeof (tests[0]) ; i++)
         {
            // Assemble test message.
            char msg[100];
            sprintf(msg, "Test %d, Contact '%s'", i, tests[i].contact_field);

            // Compose a SipMessage with the right Contact field value.
            char buffer[1000];
            sprintf(buffer, message_skeleton, tests[i].contact_field);
            SipMessage message(buffer, strlen(buffer));

            UtlString uri;
            UtlBoolean ret = message.getContactUri(0, &uri);

            // fprintf(stderr, "ret = %d, uri = '%s'\n", ret, uri.data());

            CPPUNIT_ASSERT_MESSAGE(msg, ret);
            ASSERT_STR_EQUAL_MESSAGE(msg, tests[i].contact_uri, uri.data());
         }
      };

   // Test:
   // getToField, getToUri, getToUrl
   // getFromField, getFromUri, getFromUrl
   // getContactField, getContactUri
   void testGetFieldUris()
      {
         // A single test.
         struct test
         {
            // The value to be inserted into the field.
            const char* field;
            // The value when the field is extracted, parsed into a Url,
            // and retrieved with Url::toString as a name-addr.
            const char* name_addr;
            // The value when the field is extracted, parsed into a Url,
            // and retrieved with Url::getUri as an addr-spec.
            const char* addr_spec;
         };

         // The tests
         struct test tests[] =
         {
            { "sip:example.com",
              "sip:example.com",
              "sip:example.com" },
            { "sip:100@example.com",
              "sip:100@example.com",
              "sip:100@example.com" },
            { "sip:100@example.com:100",
              "sip:100@example.com:100",
              "sip:100@example.com:100" },
            { "<sip:100@example.com;transport=udp>",
              "<sip:100@example.com;transport=udp>",
              "sip:100@example.com;transport=udp" },
            { "<sip:100@example.com>;transport=udp",
              "<sip:100@example.com>;transport=udp",
              "sip:100@example.com" },
            { "ABC<sip:100@example.com>",
              "ABC<sip:100@example.com>",
              "sip:100@example.com" },
            { "\"ABC\"<sip:100@example.com>",
              "\"ABC\"<sip:100@example.com>",
              "sip:100@example.com" },
            { "\"A<C\"<sip:100@example.com>",
              "\"A<C\"<sip:100@example.com>",
              "sip:100@example.com" },
         };

         const char* message_skeleton =
            "REGISTER sip:pingtel.com;transport=udp SIP/2.0\r\n"
            "%s: %s\r\n"
            "Call-Id: 3c26700a99cf-n3b3x9avtv3l@snom320-00041324190C\r\n"
            "Cseq: 264 REGISTER\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         char message_buffer[1000];
         UtlString value;
         Url uri;
         char msg[100];

         for (unsigned int i = 0; i < sizeof (tests) / sizeof (tests[0]); i++)
         {
            sprintf(msg, "Test %d using field value '%s'", i, tests[i].field);

            // The To field tests.
            {
               // Create the message text.
               sprintf(message_buffer, message_skeleton, "To", tests[i].field);
               // Create a SipMessage.
               SipMessage message(message_buffer);
               // getToField
               message.getToField(&value);
               ASSERT_STR_EQUAL_MESSAGE(msg, tests[i].field, value.data());
               // getToUri
               message.getToUri(&value);
               ASSERT_STR_EQUAL_MESSAGE(msg, tests[i].addr_spec, value.data());
               // getToUrl
               message.getToUrl(uri);
               uri.toString(value);
               ASSERT_STR_EQUAL_MESSAGE(msg, tests[i].name_addr, value.data());
            }

            // The From field tests.
            {
               // Create the message text.
               sprintf(message_buffer, message_skeleton, "From", tests[i].field);
               // Create a SipMessage.
               SipMessage message(message_buffer);
               // getFromField
               message.getFromField(&value);
               ASSERT_STR_EQUAL_MESSAGE(msg, tests[i].field, value.data());
               // getFromUri
               message.getFromUri(&value);
               ASSERT_STR_EQUAL_MESSAGE(msg, tests[i].addr_spec, value.data());
               // getFromUrl
               message.getFromUrl(uri);
               uri.toString(value);
               ASSERT_STR_EQUAL_MESSAGE(msg, tests[i].name_addr, value.data());
            }

            // The Contact field tests.
            {
               // Create the message text.
               sprintf(message_buffer, message_skeleton, "Contact", tests[i].field);
               // Create a SipMessage.
               SipMessage message(message_buffer);
               // getContactField
               message.getContactField(0, value);
               ASSERT_STR_EQUAL_MESSAGE(msg, tests[i].field, value.data());
               // getContactUri
               message.getContactUri(0, &value);
               ASSERT_STR_EQUAL_MESSAGE(msg, tests[i].addr_spec, value.data());
            }
         }
      };

   void testBuildSipUri()
      {
         UtlString s;

         SipMessage::buildSipUri(&s,
                                 "example.com",
                                 PORT_NONE,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL);
         ASSERT_STR_EQUAL("<sip:example.com>", s.data());

         SipMessage::buildSipUri(&s,
                                 "example.com",
                                 5060,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL);
         ASSERT_STR_EQUAL("<sip:example.com:5060>", s.data());

         SipMessage::buildSipUri(&s,
                                 "example.com",
                                 PORT_NONE,
                                 "tcp",
                                 NULL,
                                 NULL,
                                 NULL);
         ASSERT_STR_EQUAL("<sip:example.com;transport=tcp>", s.data());

         SipMessage::buildSipUri(&s,
                                 "example.com",
                                 PORT_NONE,
                                 NULL,
                                 "foo",
                                 NULL,
                                 NULL);
         ASSERT_STR_EQUAL("<sip:foo@example.com>", s.data());

         SipMessage::buildSipUri(&s,
                                 "example.com",
                                 PORT_NONE,
                                 NULL,
                                 NULL,
                                 "Name",
                                 NULL);
         ASSERT_STR_EQUAL("Name<sip:example.com>", s.data());

         SipMessage::buildSipUri(&s,
                                 "example.com",
                                 PORT_NONE,
                                 NULL,
                                 NULL,
                                 NULL,
                                 "1234");
         ASSERT_STR_EQUAL("<sip:example.com>;tag=1234", s.data());

         SipMessage::buildSipUri(&s,
                                 "example.com",
                                 9999,
                                 "udp",
                                 "foo",
                                 "Joe Blow",
                                 "abcd");
         ASSERT_STR_EQUAL("Joe Blow<sip:foo@example.com:9999;transport=udp>;tag=abcd",
                          s.data());
      };

   void testReplacesData()
      {
         UtlString callId;
         UtlString toTag;
         UtlString fromTag;

         // Check that no replaces header is found when none is there
         callId  = "staleCallId";
         toTag   = "staleToTag";
         fromTag = "staleFromTag";

         const char* messageNoReplaces =
            "INVITE sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage noReplacesMsg(messageNoReplaces, strlen(messageNoReplaces));

         CPPUNIT_ASSERT(!noReplacesMsg.getReplacesData(callId, toTag, fromTag));
         CPPUNIT_ASSERT(callId.isNull());
         CPPUNIT_ASSERT(toTag.isNull());
         CPPUNIT_ASSERT(fromTag.isNull());

         // Check that a replaces header without the required parameters is not accepted.
         callId  = "staleCallId";
         toTag   = "staleToTag";
         fromTag = "staleFromTag";

         const char* messageBadReplaces =
            "INVITE sip:sipx.local SIP/2.0\r\n"
            "Replaces: invalid-content\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         // Construct a message.
         SipMessage badReplacesMsg(messageBadReplaces, strlen(messageBadReplaces));

         // Check that no replaces header is found
         CPPUNIT_ASSERT(!badReplacesMsg.getReplacesData(callId, toTag, fromTag));
         CPPUNIT_ASSERT(callId.isNull());
         CPPUNIT_ASSERT(toTag.isNull());
         CPPUNIT_ASSERT(fromTag.isNull());

         // Check that a replaces header with only some required parameters is not accepted.
         callId  = "staleCallId";
         toTag   = "staleToTag";
         fromTag = "staleFromTag";

         const char* messageIncompleteReplaces =
            "INVITE sip:sipx.local SIP/2.0\r\n"
            "Replaces: valid@callid;from-tag=xyzzy;generic=other\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         // Construct a message.
         SipMessage incompleteReplacesMsg(messageIncompleteReplaces,
                                          strlen(messageIncompleteReplaces));

         // Check that no replaces header is found
         CPPUNIT_ASSERT(!incompleteReplacesMsg.getReplacesData(callId, toTag, fromTag));
         CPPUNIT_ASSERT(callId.isNull());
         CPPUNIT_ASSERT(toTag.isNull());
         CPPUNIT_ASSERT(fromTag.isNull());

         // Check that a replaces header with all required parameters is accepted.
         callId  = "staleCallId";
         toTag   = "staleToTag";
         fromTag = "staleFromTag";

         const char* messageWithReplaces =
            "INVITE sip:sipx.local SIP/2.0\r\n"
            "Replaces: valid@callid;from-tag=xyzzy;generic=other;to-tag=foobar\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipsend/0.01\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         // Construct a message.
         SipMessage withReplacesMsg(messageWithReplaces,
                                          strlen(messageWithReplaces));

         // Check that a replaces header is found
         CPPUNIT_ASSERT(withReplacesMsg.getReplacesData(callId, toTag, fromTag));
         ASSERT_STR_EQUAL("valid@callid", callId.data());
         ASSERT_STR_EQUAL("foobar", toTag.data());
         ASSERT_STR_EQUAL("xyzzy", fromTag.data());

         // Check that a valid replaces header is built and inserted
         SipMessage buildMsg(messageNoReplaces, strlen(messageNoReplaces));

         UtlString replacesField;
         UtlString toAddress("\"SomeBody\" <somebody@somewhere;param=a?header=b>;xy=abc;tag=good");
         UtlString fromAddress("<nobody@nowhere;param=f?header=g>;xy=def;tag=better");
         UtlString targetCallId("target@callid");

         SipMessage::buildReplacesField(replacesField, targetCallId,
                                        fromAddress.data(), toAddress.data());
         buildMsg.addHeaderField(SIP_REPLACES_FIELD, replacesField.data());

         CPPUNIT_ASSERT(buildMsg.getReplacesData(callId, toTag, fromTag));
         ASSERT_STR_EQUAL("target@callid", callId.data());
         ASSERT_STR_EQUAL("good", toTag.data());
         ASSERT_STR_EQUAL("better", fromTag.data());
      }

   void testIsClientStrictRouted()
      {
         const char* notStrictStr =
            "REFER sip:200@10.1.1.43 SIP/2.0\r\n"
            "Route: <sip:10.1.1.20:5080;lr;sipXecs-rs=%2Afrom%7EOUZDMjgyQkYtMjJBRUE2Njg%60.400_authrules%2Aauth%7E%2198bfae59c88968ac075a04ea024ae648>\r\n"
            "From: <sip:500@sukothai.pingtel.com>;tag=95632294\r\n"
            "To: \"Poly One\"<sip:200@sukothai.pingtel.com>;tag=9FC282BF-22AEA668\r\n"
            "Call-Id: 438a5a3b-f60da375-e8251a76@10.1.1.43\r\n"
            "Cseq: 3 REFER\r\n"
            "Contact: sip:10.1.1.20:5120\r\n"
            "Referred-By: <sip:500@sukothai.pingtel.com>\r\n"
            "Refer-To: <sip:201@sukothai.pingtel.com>;transport=udp\r\n"
            "Date: Mon, 15 Oct 2007 15:30:03 GMT\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipX/3.9.3 (Linux)\r\n"
            "Accept-Language: en\r\n"
            "Allow: INVITE, ACK, CANCEL, BYE, REFER, OPTIONS, NOTIFY, SUBSCRIBE\r\n"
            "Supported: replaces\r\n"
            "Via: SIP/2.0/UDP 10.1.1.20:5120;branch=z9hG4bK-sipX-0007a7093deef33050d83231a89b3d79693b\r\n"
            "Content-Length: 0\r\n"
            "\r\n"
            ;
         SipMessage notStrict(notStrictStr);

         CPPUNIT_ASSERT(!notStrict.isClientMsgStrictRouted());

         const char* strictStr =
            "REFER sip:200@10.1.1.43 SIP/2.0\r\n"
            "Route: <sip:10.1.1.20:5080;sipXecs-rs=%2Afrom%7EOUZDMjgyQkYtMjJBRUE2Njg%60.400_authrules%2Aauth%7E%2198bfae59c88968ac075a04ea024ae648>\r\n"
            "From: <sip:500@sukothai.pingtel.com>;tag=95632294\r\n"
            "To: \"Poly One\"<sip:200@sukothai.pingtel.com>;tag=9FC282BF-22AEA668\r\n"
            "Call-Id: 438a5a3b-f60da375-e8251a76@10.1.1.43\r\n"
            "Cseq: 3 REFER\r\n"
            "Contact: sip:10.1.1.20:5120\r\n"
            "Referred-By: <sip:500@sukothai.pingtel.com>\r\n"
            "Refer-To: <sip:201@sukothai.pingtel.com>;transport=udp\r\n"
            "Date: Mon, 15 Oct 2007 15:30:03 GMT\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: sipX/3.9.3 (Linux)\r\n"
            "Accept-Language: en\r\n"
            "Allow: INVITE, ACK, CANCEL, BYE, REFER, OPTIONS, NOTIFY, SUBSCRIBE\r\n"
            "Supported: replaces\r\n"
            "Via: SIP/2.0/UDP 10.1.1.20:5120;branch=z9hG4bK-sipX-0007a7093deef33050d83231a89b3d79693b\r\n"
            "Content-Length: 0\r\n"
            "\r\n"
            ;
         SipMessage strict(strictStr);

         CPPUNIT_ASSERT(strict.isClientMsgStrictRouted());
      }

   void testRecordRoutesAccepted()
      {
      const char* inviteMessage =
          "INVITE sip:sipx.local SIP/2.0\r\n"
          "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 INVITE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage inviteSipMsg(inviteMessage, strlen(inviteMessage));
      CPPUNIT_ASSERT( inviteSipMsg.isRecordRouteAccepted() == TRUE );

      const char* ackMessage =
          "ACK sip:sipx.local SIP/2.0\r\n"
          "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 ACK\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage ackSipMsg(ackMessage, strlen(ackMessage));
      CPPUNIT_ASSERT( ackSipMsg.isRecordRouteAccepted() == TRUE );

      const char* byeMessage =
          "BYE sip:sipx.local SIP/2.0\r\n"
          "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 BYE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage byeSipMsg(byeMessage, strlen(byeMessage));
      CPPUNIT_ASSERT( byeSipMsg.isRecordRouteAccepted() == TRUE );

      const char* cancelMessage =
          "CANCEL sip:sipx.local SIP/2.0\r\n"
          "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 CANCEL\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage cancelSipMsg(cancelMessage, strlen(cancelMessage));
      CPPUNIT_ASSERT( cancelSipMsg.isRecordRouteAccepted() == TRUE );

      const char* optionsMessage =
          "OPTIONS sip:sipx.local SIP/2.0\r\n"
          "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 OPTIONS\r\n"
          "Max-Forwards: 20\r\n"
          "Supported: replaces\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage optionsSipMsg(optionsMessage, strlen(optionsMessage));
      CPPUNIT_ASSERT( optionsSipMsg.isRecordRouteAccepted() == TRUE );

      const char* registerMessage =
          "REGISTER sip:sipx.local SIP/2.0\r\n"
          "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
          "From: <sip:test@sipx.local>;tag=94b99ae0-2f816f01-13c4-be5-50f2a8d5-be5\r\n"
          "To: <sip:test@sipx.local>\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 REGISTER\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: test@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage registerSipMsg(registerMessage, strlen(registerMessage));
      CPPUNIT_ASSERT( registerSipMsg.isRecordRouteAccepted() == FALSE );

      const char* infoMessage =
          "INFO sip:sipx.local SIP/2.0\r\n"
          "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 INFO\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Supported: timer\r\n"
          "Content-Type: application/dtmf-relay\r\n"
          "Content-Length: 26\r\n"
          "\r\n"
          "Signal= 1\r\n"
          "Duration= 160\r\n"
          "\r\n";

      SipMessage infoSipMsg(infoMessage, strlen(infoMessage));
      CPPUNIT_ASSERT( infoSipMsg.isRecordRouteAccepted() == TRUE );

      const char* prackMessage =
          "PRACK sip:sipx.local SIP/2.0\r\n"
          "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 PRACK\r\n"
          "Max-Forwards: 20\r\n"
          "RAck: 1 2 INVITE\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage prackSipMsg(prackMessage, strlen(prackMessage));
      CPPUNIT_ASSERT( prackSipMsg.isRecordRouteAccepted() == TRUE );

      const char* subscribeMessage =
          "SUBSCRIBE sip:sipx.local SIP/2.0\r\n"
          "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 SUBSCRIBE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Expires: 3600\r\n"
          "Event: message-summary\r\n"
          "Max-Forwards: 20\r\n"
          "Supported: replaces\r\n"
          "Accept: application/simple-message-summary\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage subscribeSipMsg(subscribeMessage, strlen(subscribeMessage));
      CPPUNIT_ASSERT( subscribeSipMsg.isRecordRouteAccepted() == TRUE );

      const char* notifyMessage =
          "NOTIFY sip:sipx.local SIP/2.0\r\n"
          "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 NOTIFY\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Event: message-summary\r\n"
          "Content-Type: application/simple-message-summary\r\n"
          "Content-Length: 50\r\n"
          "\r\n"
          "Messages-Waiting: no\r\n"
          "Voice-Message: 0/0 (0/0)\r\n"
          "\r\n";

      SipMessage notifySipMsg(notifyMessage, strlen(notifyMessage));
      CPPUNIT_ASSERT( notifySipMsg.isRecordRouteAccepted() == TRUE );

      const char* updateMessage =
          "UPDATE sip:sipx.local SIP/2.0\r\n"
          "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 UPDATE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage updateSipMsg(updateMessage, strlen(updateMessage));
      CPPUNIT_ASSERT( updateSipMsg.isRecordRouteAccepted() == TRUE );

      const char* messageMessage =
          "MESSAGE sip:sipx.local SIP/2.0\r\n"
          "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 MESSAGE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage messageSipMsg(messageMessage, strlen(messageMessage));
      CPPUNIT_ASSERT( messageSipMsg.isRecordRouteAccepted() == FALSE );

      const char* referMessage =
          "REFER sip:sipx.local SIP/2.0\r\n"
          "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 REFER\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage referSipMsg(referMessage, strlen(referMessage));
      CPPUNIT_ASSERT( referSipMsg.isRecordRouteAccepted() == TRUE );

      const char* publishMessage =
          "PUBLISH sip:sipx.local SIP/2.0\r\n"
          "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 PUBLISH\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage publishSipMsg(publishMessage, strlen(publishMessage));
      CPPUNIT_ASSERT( publishSipMsg.isRecordRouteAccepted() == FALSE );

      }

   void testPathFieldManipulations()
   {
      // try to add a Route header when there is already one present
      const char* rawmsg =
         "REGISTER sip:sipx.local SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "From: <sip:test@sipx.local>;tag=94b99ae0-2f816f01-13c4-be5-50f2a8d5-be5\r\n"
         "To: <sip:test@sipx.local>\r\n"
         "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 REGISTER\r\n"
         "Path: <sip:anothermiddleproxy.com>, <sip:middleproxy.com>\r\n"
         "Path: <sip:outboundproxy.com>\r\n"
         "Max-Forwards: 20\r\n"
         "Contact: test@127.0.0.1\r\n"
         "Content-Length: 0\r\n"
         "\r\n";

      SipMessage sipmsg(rawmsg, strlen(rawmsg));
      sipmsg.addPathUri("sip:homeproxy.com");
      sipmsg.addLastPathUri("sip:last.com");
      CPPUNIT_ASSERT_EQUAL(3, sipmsg.getCountHeaderFields("Path"));

      UtlString tmpString;
      CPPUNIT_ASSERT(sipmsg.getPathUri(0, &tmpString)==TRUE);
      ASSERT_STR_EQUAL("<sip:homeproxy.com>", tmpString.data() );

      CPPUNIT_ASSERT(sipmsg.getPathUri(1, &tmpString)==TRUE);
      ASSERT_STR_EQUAL("<sip:anothermiddleproxy.com>", tmpString.data() );

      CPPUNIT_ASSERT(sipmsg.getPathUri(2, &tmpString)==TRUE);
      ASSERT_STR_EQUAL("<sip:middleproxy.com>", tmpString.data() );

      CPPUNIT_ASSERT(sipmsg.getPathUri(3, &tmpString)==TRUE);
      ASSERT_STR_EQUAL("<sip:outboundproxy.com>", tmpString.data() );

      CPPUNIT_ASSERT(sipmsg.getPathUri(4, &tmpString)==TRUE);
      ASSERT_STR_EQUAL("<sip:last.com>", tmpString.data() );

      CPPUNIT_ASSERT(sipmsg.getPathUri(5, &tmpString)==FALSE);

   }

   void testRouteFieldManipulations()
   {
      // try to add a Route header when there is already one present
      const char* rawmsg =
         "INVITE sip:sipx.local SIP/2.0\r\n"
         "Route: <sip:anothermiddleproxy.com>, <sip:middleproxy.com>\r\n"
         "Route: <sip:outboundproxy.com>\r\n"
         "To: sip:sipx.local\r\n"
         "From: Sip Send <sip:sender@example.org>; tag=ORIG-TAG\r\n"
         "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 20\r\n"
         "Contact: me@127.0.0.1\r\n"
         "Content-Length: 0\r\n"
         "\r\n";

      SipMessage sipmsg(rawmsg, strlen(rawmsg));
      sipmsg.addRouteUri("sip:homeproxy.com");
      sipmsg.addLastRouteUri("sip:last.com");
      CPPUNIT_ASSERT_EQUAL(3, sipmsg.getCountHeaderFields("Route"));

      UtlString tmpString;
      CPPUNIT_ASSERT(sipmsg.getRouteUri(0, &tmpString)==TRUE);
      ASSERT_STR_EQUAL("<sip:homeproxy.com>", tmpString.data() );

      CPPUNIT_ASSERT(sipmsg.getRouteUri(1, &tmpString)==TRUE);
      ASSERT_STR_EQUAL("<sip:anothermiddleproxy.com>", tmpString.data() );

      CPPUNIT_ASSERT(sipmsg.getRouteUri(2, &tmpString)==TRUE);
      ASSERT_STR_EQUAL("<sip:middleproxy.com>", tmpString.data() );

      CPPUNIT_ASSERT(sipmsg.getRouteUri(3, &tmpString)==TRUE);
      ASSERT_STR_EQUAL("<sip:outboundproxy.com>", tmpString.data() );

      CPPUNIT_ASSERT(sipmsg.getRouteUri(4, &tmpString)==TRUE);
      ASSERT_STR_EQUAL("<sip:last.com>", tmpString.data() );

      CPPUNIT_ASSERT(sipmsg.getRouteUri(5, &tmpString)==FALSE);
   }

   void test200ResponseToRequestWithPath()
   {
      SipMessage finalResponse;

      const char* badReferMsg =
         "REFER sip:sipx.local SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:sipx.local\r\n"
         "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 REFER\r\n"
         "Path: <sip:anothermiddleproxy.com>, <sip:middleproxy.com>\r\n"
         "Path: <sip:outboundproxy.com>\r\n"
         "Max-Forwards: 20\r\n"
         "Contact: me@127.0.0.1\r\n"
         "Content-Length: 0\r\n"
         "\r\n";

      // Only REGISTER requests should be allowed to contain path headers.
      // Verify that 200 OK to non-REGISTER request does not contain a copy
      // of the Path headers

      SipMessage badReferSipMsg(badReferMsg, strlen(badReferMsg));
      finalResponse.setOkResponseData(&badReferSipMsg);
      CPPUNIT_ASSERT_EQUAL(0, finalResponse.getCountHeaderFields("Path"));

      // try to add a Route header when there is already one present
      const char* goodRegisterMsg =
         "REGISTER sip:sipx.local SIP/2.0\r\n"
          "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
          "From: <sip:test@sipx.local>;tag=94b99ae0-2f816f01-13c4-be5-50f2a8d5-be5\r\n"
          "To: <sip:test@sipx.local>\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 REGISTER\r\n"
          "Path: <sip:anothermiddleproxy.com>, <sip:middleproxy.com>\r\n"
          "Path: <sip:outboundproxy.com>\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: test@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage goodRegisterSipMsg(goodRegisterMsg, strlen(goodRegisterMsg));
      finalResponse.setOkResponseData(&goodRegisterSipMsg);
      CPPUNIT_ASSERT_EQUAL(2, finalResponse.getCountHeaderFields("Path"));

      UtlString tmpString;
      CPPUNIT_ASSERT(finalResponse.getPathUri(0, &tmpString)==TRUE);
      ASSERT_STR_EQUAL("<sip:anothermiddleproxy.com>", tmpString.data() );

      CPPUNIT_ASSERT(finalResponse.getPathUri(1, &tmpString)==TRUE);
      ASSERT_STR_EQUAL("<sip:middleproxy.com>", tmpString.data() );

      CPPUNIT_ASSERT(finalResponse.getPathUri(2, &tmpString)==TRUE);
      ASSERT_STR_EQUAL("<sip:outboundproxy.com>", tmpString.data() );

      CPPUNIT_ASSERT(finalResponse.getPathUri(3, &tmpString)==FALSE);
   }

   void testSipXNatRoute()
   {
       const char* InviteMsg =
          "INVITE sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:sender@example.org>; tag=ORIG-TAG\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 INVITE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

       SipMessage sipmsg( InviteMsg, strlen( InviteMsg ) );
       const UtlString temporaryRoute( "47.135.162.145:10491;transport=udp" );
       UtlString getResult;

       // message does not contain a SipX NAT Route - attempt to get it must fail
       CPPUNIT_ASSERT( sipmsg.getSipXNatRoute( &getResult ) == false );
       CPPUNIT_ASSERT( getResult.isNull() );

       // set the SipX NAT Route and try to read it back
       sipmsg.setSipXNatRoute( temporaryRoute );
       CPPUNIT_ASSERT( sipmsg.getSipXNatRoute( &getResult ) == true );
       CPPUNIT_ASSERT( !getResult.isNull() );
       CPPUNIT_ASSERT( getResult.compareTo( temporaryRoute ) == 0 );

       // remove the SipX NAT Route - attempt to get it must fail
       sipmsg.removeSipXNatRoute();
       CPPUNIT_ASSERT( sipmsg.getSipXNatRoute( &getResult ) == false );
       CPPUNIT_ASSERT( getResult.isNull() );
   }

   void testGetFieldSubfield()
   {
      // Basic test of SipMessage::getFieldSubfield.
      // This test avoids many of the difficult cases (which getFieldSubfield
      // does not handle correctly).
      const char *msg_string =
         "REGISTER sip:sipx.local SIP/2.0\r\n"
         "Supported: a,b\r\n"   // No space in "a,b" to prevent space in output.
         "Supported: c\r\n"
         "\r\n";
      SipMessage msg(msg_string);

      UtlString value;
      CPPUNIT_ASSERT(msg.getFieldSubfield(SIP_SUPPORTED_FIELD, 0, &value));
      ASSERT_STR_EQUAL("a", value.data());
      CPPUNIT_ASSERT(msg.getFieldSubfield(SIP_SUPPORTED_FIELD, 1, &value));
      ASSERT_STR_EQUAL("b", value.data());
      CPPUNIT_ASSERT(msg.getFieldSubfield(SIP_SUPPORTED_FIELD, 2, &value));
      ASSERT_STR_EQUAL("c", value.data());
      CPPUNIT_ASSERT(!msg.getFieldSubfield(SIP_SUPPORTED_FIELD, 3, &value));
      CPPUNIT_ASSERT(msg.getFieldSubfield(SIP_SUPPORTED_FIELD, BOTTOM_SUBFIELD, &value));
      ASSERT_STR_EQUAL("c", value.data());
   }

   void testSetFieldSubfield_SingleSubfields()
   {
      const char* InviteMsg =
         "INVITE sip:sipx.local SIP/2.0\r\n"
         "DummyHeader: subfield 0\r\n"
         "To: sip:sipx.local\r\n"
         "DummyHeader: subfield 1\r\n"
         "From: Sip Send <sip:sender@example.org>; tag=ORIG-TAG\r\n"
         "DummyHeader: subfield 2\r\n"
         "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "DummyHeader: subfield 3\r\n"
         "Cseq: 1 INVITE\r\n"
         "DummyHeader: subfield 4\r\n"
         "Max-Forwards: 20\r\n"
         "DummyHeader: subfield 5\r\n"
         "Contact: me@127.0.0.1\r\n"
         "DummyHeader: subfield 6\r\n"
         "Content-Length: 0\r\n"
         "DummyHeader: subfield 7\r\n"
         "\r\n";

      SipMessage sipmsg( InviteMsg, strlen( InviteMsg ) );

      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 0, "How About This Value") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 1, "Or That One") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 2, "Maybe This One Then") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 3, "Im Serious") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 4, "Ok Thats Enough") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 5, "What Else") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 6, "Im Asking") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 7, "Never Mind") );
      CPPUNIT_ASSERT(!sipmsg.setFieldSubfield( "DummyHeader", 8, "Forget About It") );

      const char* ReferenceMsg =
         "INVITE sip:sipx.local SIP/2.0\r\n"
         "DummyHeader: How About This Value\r\n"
         "To: sip:sipx.local\r\n"
         "DummyHeader: Or That One\r\n"
         "From: Sip Send <sip:sender@example.org>; tag=ORIG-TAG\r\n"
         "DummyHeader: Maybe This One Then\r\n"
         "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "DummyHeader: Im Serious\r\n"
         "Cseq: 1 INVITE\r\n"
         "DummyHeader: Ok Thats Enough\r\n"
         "Max-Forwards: 20\r\n"
         "DummyHeader: What Else\r\n"
         "Contact: me@127.0.0.1\r\n"
         "DummyHeader: Im Asking\r\n"
         "Content-Length: 0\r\n"
         "DummyHeader: Never Mind\r\n"
         "\r\n";

      SipMessage refsipmsg( ReferenceMsg, strlen( ReferenceMsg ) );

      UtlString modifiedMsgString, referenceMsgString;
      ssize_t msgLen;
      sipmsg.getBytes(&modifiedMsgString, &msgLen);
      refsipmsg.getBytes(&referenceMsgString, &msgLen);

      ASSERT_STR_EQUAL(referenceMsgString.data(), modifiedMsgString.data());
   }
   void testSetFieldSubfield_Mixed()
   {

      const char* InviteMsg =
         "INVITE sip:sipx.local SIP/2.0\r\n"
         "DummyHeader: subfield 0,   subfield 1, subfield 2\r\n"
         "To: sip:sipx.local\r\n"
         "DummyHeader: subfield3\r\n"
         "From: Sip Send <sip:sender@example.org>; tag =ORIG-TAG\r\n"
         "DummyHeader: subfield 4,       subfield 5,    subfield 6\r\n"
         "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "DummyHeader: subfield7\r\n"
         "Cseq: 1 INVITE\r\n"
         "DummyHeader: subfield 8    ,    subfield 9 ,subfield 10\r\n"
         "Max-Forwards: 20\r\n"
         "DummyHeader: subfield11\r\n"
         "Contact: me@127.0.0.1\r\n"
         "DummyHeader: subfield 12  ,  subfield 13,  subfield 14\r\n"
         "Content-Length: 0\r\n"
         "DummyHeader: subfield 15\r\n"
         "\r\n";

      SipMessage sipmsg( InviteMsg, strlen( InviteMsg ) );

      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 0, "etre") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 1, "paraitre") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 2, "sembler") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 3, "demeurer") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 4, "rester") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 5, "passerpour") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 6, "mais") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 7, "ou") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 8, "et") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 9, "donc") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 10, "car") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 11, "ni") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 12, "or") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 13, "bonne") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 14, "question") );
      CPPUNIT_ASSERT(sipmsg.setFieldSubfield( "DummyHeader", 15, "Charlemagne") );
      CPPUNIT_ASSERT(!sipmsg.setFieldSubfield( "DummyHeader", 16, "Charlemagne") );

      const char* ReferenceMsg =
         "INVITE sip:sipx.local SIP/2.0\r\n"
         "DummyHeader: etre,paraitre,sembler\r\n"
         "To: sip:sipx.local\r\n"
         "DummyHeader: demeurer\r\n"
         "From: Sip Send <sip:sender@example.org>; tag =ORIG-TAG\r\n"
         "DummyHeader: rester,passerpour,mais\r\n"
         "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "DummyHeader: ou\r\n"
         "Cseq: 1 INVITE\r\n"
         "DummyHeader: et,donc,car\r\n"
         "Max-Forwards: 20\r\n"
         "DummyHeader: ni\r\n"
         "Contact: me@127.0.0.1\r\n"
         "DummyHeader: or,bonne,question\r\n"
         "Content-Length: 0\r\n"
         "DummyHeader: Charlemagne\r\n"
         "\r\n";

      SipMessage refsipmsg( ReferenceMsg, strlen( ReferenceMsg ) );

      UtlString modifiedMsgString, referenceMsgString;
      ssize_t msgLen;
      sipmsg.getBytes(&modifiedMsgString, &msgLen);
      refsipmsg.getBytes(&referenceMsgString, &msgLen);

      ASSERT_STR_EQUAL(referenceMsgString.data(), modifiedMsgString.data());
   }

   void testSetViaTag()
   {
      const char* SimpleMessage =
         "REGISTER sip:sipx.local SIP/2.0\r\n"
         "Via: SIP/2.0/TCP sipx.local1:33855;branch=z9hG4bK-10ca3d;test=1\r\n"
         "Via: SIP/2.0/UDP sipx.local2:3355;branch=z9hG4bK-10cb6f93ea3d;test=2\r\n"
         "Via: SIP/2.0/UDP sipx.local3:3385;branch=z9hG4bK-10cb6f938a;test=3\r\n"
         "Via: SIP/2.0/TCP sipx.local4:1234;branch=z9hG4bK-10cb6f;test=4,SIP/2.0/UDP sipx.local5:1312;branch=z9hG4bK-10cb6f9378;test=5\r\n"
         "Via: SIP/2.0/UDP sipx.local6:6546;branch=z9hG4bK-10cb6f9310ef4dc78ea3d;test=6\r\n"
         "Via: SIP/2.0/TCP sipx.local7:4372;branch=z9hG4bK-10cb6f98e10ef4dc78ea3d;test=7\r\n"
         "To: sip:sipx.local\r\n"
         "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 REGISTER\r\n"
         "Max-Forwards: 20\r\n"
         "User-Agent: sipsend/0.01\r\n"
         "Contact: me@127.0.0.1\r\n"
         "Expires: 300\r\n"
         "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

      CPPUNIT_ASSERT( testMsg.setViaTag( "new1", "test", 0 ) );
      CPPUNIT_ASSERT( testMsg.setViaTag( "12", "test2", 0 ) );
      CPPUNIT_ASSERT( testMsg.setViaTag( "new2", "test", 1 ) );
      CPPUNIT_ASSERT( testMsg.setViaTag( "22", "test2", 1 ) );
      CPPUNIT_ASSERT( testMsg.setViaTag( "new3", "test", 2 ) );
      CPPUNIT_ASSERT( testMsg.setViaTag( "32", "test2", 2 ) );
      CPPUNIT_ASSERT( testMsg.setViaTag( "new4", "test", 3 ) );
      CPPUNIT_ASSERT( testMsg.setViaTag( "42", "test2", 3 ) );
      CPPUNIT_ASSERT( testMsg.setViaTag( "new5", "test", 4 ) );
      CPPUNIT_ASSERT( testMsg.setViaTag( "52", "test2", 4 ) );
      CPPUNIT_ASSERT( testMsg.setViaTag( "new6", "test", 5 ) );
      CPPUNIT_ASSERT( testMsg.setViaTag( "62", "test2", 5 ) );
      CPPUNIT_ASSERT( testMsg.setViaTag( "new7", "test", 6 ) );
      CPPUNIT_ASSERT( testMsg.setViaTag( "72", "test2", 6 ) );
      CPPUNIT_ASSERT( !testMsg.setViaTag( "new7", "test", 7 ) );
      CPPUNIT_ASSERT( !testMsg.setViaTag( "72", "test2", 7 ) );

      const char* ReferenceMessage =
         "REGISTER sip:sipx.local SIP/2.0\r\n"
         "Via: SIP/2.0/TCP sipx.local1:33855;branch=z9hG4bK-10ca3d;test=new1;test2=12\r\n"
         "Via: SIP/2.0/UDP sipx.local2:3355;branch=z9hG4bK-10cb6f93ea3d;test=new2;test2=22\r\n"
         "Via: SIP/2.0/UDP sipx.local3:3385;branch=z9hG4bK-10cb6f938a;test=new3;test2=32\r\n"
         "Via: SIP/2.0/TCP sipx.local4:1234;branch=z9hG4bK-10cb6f;test=new4;test2=42,SIP/2.0/UDP sipx.local5:1312;branch=z9hG4bK-10cb6f9378;test=new5;test2=52\r\n"
         "Via: SIP/2.0/UDP sipx.local6:6546;branch=z9hG4bK-10cb6f9310ef4dc78ea3d;test=new6;test2=62\r\n"
         "Via: SIP/2.0/TCP sipx.local7:4372;branch=z9hG4bK-10cb6f98e10ef4dc78ea3d;test=new7;test2=72\r\n"
         "To: sip:sipx.local\r\n"
         "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 REGISTER\r\n"
         "Max-Forwards: 20\r\n"
         "User-Agent: sipsend/0.01\r\n"
         "Contact: me@127.0.0.1\r\n"
         "Expires: 300\r\n"
         "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage refMsg( ReferenceMessage, strlen( ReferenceMessage ) );

      UtlString modifiedMsgString, referenceMsgString;
      ssize_t msgLen;
      testMsg.getBytes(&modifiedMsgString, &msgLen);
      refMsg.getBytes(&referenceMsgString, &msgLen);

      ASSERT_STR_EQUAL(referenceMsgString.data(), modifiedMsgString.data());
   }

   void testRecordRouteEchoing()
   {
      const char* message1 =
         "INVITE sip:user@somewhere.com SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:user@somewhere.com\r\n"
         "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 INVITE\r\n"
         "Record-Route: <sip:myhost.example.com;lr>\r\n"
         "Record-Route: <sip:myhost2a.example.com;lr>,<sip:myhost2b.example.com;lr>,<sip:myhost2c.example.com;lr>\r\n"
         "Record-Route: <sip:myhost3.example.com;lr>\r\n"
         "Max-Forwards: 20\r\n"
         "Contact: caller@127.0.0.1\r\n"
         "Record-Route: <sip:myhost4.example.com;lr>\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage sipRequest(message1, strlen(message1));

      SipMessage response1;
      UtlString recordRouteField;
      UtlString text("blah");
      response1.setResponseData(&sipRequest, 400, text.data() );
      CPPUNIT_ASSERT( response1.getRecordRouteField(0, &recordRouteField ) );
      ASSERT_STR_EQUAL( "<sip:myhost.example.com;lr>", recordRouteField.data() );
      CPPUNIT_ASSERT( response1.getRecordRouteField(1, &recordRouteField ) );
      ASSERT_STR_EQUAL( "<sip:myhost2a.example.com;lr>,<sip:myhost2b.example.com;lr>,<sip:myhost2c.example.com;lr>", recordRouteField.data() );
      CPPUNIT_ASSERT( response1.getRecordRouteField(2, &recordRouteField ) );
      ASSERT_STR_EQUAL( "<sip:myhost3.example.com;lr>", recordRouteField.data() );
      CPPUNIT_ASSERT( response1.getRecordRouteField(3, &recordRouteField ) );
      ASSERT_STR_EQUAL( "<sip:myhost4.example.com;lr>", recordRouteField.data() );
      CPPUNIT_ASSERT( !response1.getRecordRouteField(4, &recordRouteField ) );

      SipMessage response2;
      UtlString localContact("<sip:bob@bobnet.bob>");
      response2.setResponseData(&sipRequest, 400, text.data(), localContact.data(), FALSE );
      CPPUNIT_ASSERT( !response2.getRecordRouteField(0, &recordRouteField ) );
   }

   void testDialogMatching()
   {
      const char* message1 =
         "OPTIONS sip:user@somewhere.com SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:user@somewhere.com; tag=30543asdkfkasjdklfjkledd3295b\r\n"
         "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 OPTIONS\r\n"
         "Record-Route: <sip:myhost.example.com;lr>\r\n"
         "Max-Forwards: 20\r\n"
         "Contact: caller@127.0.0.1\r\n"
         "Record-Route: <sip:myhost4.example.com;lr>\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage sipRequestOne(message1, strlen(message1));

      const char* message2 =
         "INVITE sip:user@somewhere.com SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:nouser@nowhere.ca; tag=30543asdkfkasjdklfjkledd3295b\r\n"
         "From: Anon <sip:nobody@nowhere.ca>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 13 INVITE\r\n"
         "Record-Route: <sip:myhost.example.com;lr>\r\n"
         "Max-Forwards: 20\r\n"
         "Contact: nobody@example.com\r\n"
         "Record-Route: <sip:myhost4.example.com;lr>\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      SipMessage sipRequestTwo(message2, strlen(message2));

      SipMessage sipRequestThree(sipRequestTwo);
      sipRequestThree.setRawToField("sip:user@somewhere.com; tag=30543asdkfkasjdklfjkledd3295b");
      sipRequestThree.setRawFromField("Caller <sip:caller@example.org>; tag=30543f3483e1cb");

      SipMessage sipRequestFour(sipRequestTwo);
      sipRequestFour.setRawToField("sip:user@somewhere.com");
      sipRequestFour.setRawFromField("Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b");

      SipMessage sipRequestFive(sipRequestTwo);
      sipRequestFive.setRawToField("sip:user@somewhere.com");
      sipRequestFive.setRawFromField("Caller <sip:caller@example.org>");

      SipMessage sipRequestSix(sipRequestTwo);
      sipRequestSix.setRawToField("Callee <sip:user@somewhere.com>; tag=30543f3483e1cb11ecb40866edd3295b");
      sipRequestSix.setRawFromField("Caller <sip:caller@example.org>");

      SipMessage sipRequestSeven(sipRequestTwo);
      sipRequestSeven.setCallIdField("123412341234");

      SipMessage sipRequestEight(sipRequestTwo);
      sipRequestEight.setRawToField("Callee <sip:user@somewhere.com>; tag=30543f3483e1cb11ecb40866edd3295b");
      sipRequestEight.setRawFromField("Caller <sip:caller@ext.example.org>");

      // Test two in-dialog requests from the same dialog, with
      // differing from and to fields
      CPPUNIT_ASSERT_MESSAGE("Failed matching in-dialog requests from the same dialog, "
                             "with differing from and to fields",
                             sipRequestOne.isSameSession(&sipRequestTwo));

      // Test two in-dialog requests from different dialogs
      CPPUNIT_ASSERT_MESSAGE("Failed not matching in-dialog requests from different dialogs",
                             !sipRequestOne.isSameSession(&sipRequestThree));

      // Test an in-dialog and dialog-forming request from the
      // same dialog
      CPPUNIT_ASSERT_MESSAGE("Failed matching an in-dialog request and a dialog-forming request from the "
                             "same dialog", sipRequestOne.isSameSession(&sipRequestFour));

      // Test backward compatibility with RFC 2543 - matching dialogs
      CPPUNIT_ASSERT_MESSAGE("Failed backward compatibility with RFC 2543 - matching dialogs",
                             sipRequestFive.isSameSession(&sipRequestSix));

      // Test matching two requests with different call-ids
      CPPUNIT_ASSERT_MESSAGE("Failed not matching two requests with different call-ids",
                             !sipRequestOne.isSameSession(&sipRequestSeven));

      // Test backward compatibility with RFC 2543 - different dialogs
      CPPUNIT_ASSERT_MESSAGE("Failed backward compatibility with RFC 2543 - different dialogs",
                             !sipRequestFive.isSameSession(&sipRequestEight));
   }

   void testGetReferencesField()
      {
         const char* SimpleMessage =
         "INVITE sip:user@somewhere.com SIP/2.0\r\n"
         "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
         "To: sip:nouser@nowhere.ca; tag=30543asdkfkasjdklfjkledd3295b\r\n"
         "From: Anon <sip:nobody@nowhere.ca>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 13 INVITE\r\n"
         "Record-Route: <sip:myhost.example.com;lr>\r\n"
         "Max-Forwards: 20\r\n"
         "Contact: nobody@example.com\r\n"
         "References: abcdef-ghijk-lmnop@example.com;rel=xfer\r\n"
         "Record-Route: <sip:myhost4.example.com;lr>\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
         SipMessage sipRequest(SimpleMessage, strlen(SimpleMessage));

         UtlString references;

         sipRequest.getReferencesField(&references);

         ASSERT_STR_EQUAL("abcdef-ghijk-lmnop@example.com;rel=xfer",references.data());
      };

   void testGetCSeqField()
      {
         // A single test.
         struct test
         {
            // The value to be inserted into the CSeq field.
            const char* field;
            // Sequence number, or -1 if it cannot be extracted.
            int seq_no;
            // Method, or "*" if it cannot be extracted.
            const char* method;
         };

#define LONG_STRING \
         "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" \
         "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" \
         "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" \
         "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

         // The tests
         struct test tests[] =
         {
            { "1234 REGISTER", 1234, "REGISTER" },
            { LONG_STRING, -1, "*" }, // XX-7979
            { "REGISTER", -1, "*" },
            { "  1234 REGISTER  ", 1234, "REGISTER" },
            { "X REGISTER", -1, "REGISTER" },
            { "123412341234 REGISTER", -1, "REGISTER" },
            { "1234", -1, "*" }, // Whitespace after seq. no. must be present.
            { "1234 ", -1, "*" }, // SipMessage::getHeaderValue trims trailing whitespace.
            { "", -1, "*" },
            { "-1 REGISTER", -1, "REGISTER" },
            { "-2 REGISTER", -1, "REGISTER" },
         };

         const char* message_skeleton =
            "REGISTER sip:pingtel.com;transport=udp SIP/2.0\r\n"
            "%s: %s\r\n"
            "Call-Id: 3c26700a99cf-n3b3x9avtv3l@snom320-00041324190C\r\n"
            "Cseq: 264 REGISTER\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         char message_buffer[1000];
         char msg[100];
         int ret;
         int seqNum;
         UtlString method;
         int should_succeed;

         for (unsigned int i = 0; i < sizeof (tests) / sizeof (tests[0]); i++)
         {
            sprintf(msg, "Test %d using field value '%s'", i, tests[i].field);

            // Create the message text.
            sprintf(message_buffer, message_skeleton, "CSeq", tests[i].field);
            // Create a SipMessage.
            SipMessage message(message_buffer);

            // Get only the sequence part.
            ret = message.getCSeqField(&seqNum, NULL);
            should_succeed = tests[i].seq_no != -1;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, should_succeed, ret);
            if (should_succeed)
            {
               CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, tests[i].seq_no, seqNum);
            }

            // Get only the method part.
            ret = message.getCSeqField(NULL, &method);
            should_succeed = strcmp(tests[i].method, "*") != 0;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg,
                                         should_succeed,
                                         ret);
            if (should_succeed)
            {
               ASSERT_STR_EQUAL_MESSAGE(msg, tests[i].method, method);
            }

            // Get both parts.
            ret = message.getCSeqField(&seqNum, &method);
            should_succeed =
               tests[i].seq_no != -1 &&
               strcmp(tests[i].method, "*") != 0;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, should_succeed, ret);
            if (should_succeed)
            {
               CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, tests[i].seq_no, seqNum);
               ASSERT_STR_EQUAL_MESSAGE(msg, tests[i].method, method);
            }
         }
      };

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipMessageTest);
