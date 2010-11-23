//
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

#include "os/OsDefs.h"
#include "os/OsConfigDb.h"
#include "net/Url.h"
#include "RouteState.h"
#include "NatTraversalAgent.h"
#include "CallTracker.h"

//EXTERNAL FUNCTIONS
void SetFakeReceptionOfPacketsFlag( bool bFakeReceptionOfPackets );

class NatTraversalAgentTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(NatTraversalAgentTest);
      CPPUNIT_TEST( handleRemoteNATedToRemoteNATedCall );
      CPPUNIT_TEST( handleLocalNATedToRemoteNATedCall );
      CPPUNIT_TEST( stalledMediaStreamDetectionTest_StreamNotStalled );
      CPPUNIT_TEST( stalledMediaStreamDetectionTest_StreamStalled );
      CPPUNIT_TEST( UndoChangesToRequestUriTests );
      CPPUNIT_TEST_SUITE_END();

   public:
      NatTraversalAgent* pNatTraversalAgent;

      void setUp()
      {
         UtlString name("ntap");
         RouteState::setSecret("fixed");
         pNatTraversalAgent = new NatTraversalAgent( name );

         OsConfigDb configuration;
         configuration.set("NATRULES", TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules3.xml");
         pNatTraversalAgent->readConfig( configuration );
      }

      void tearDown()
      {
         delete pNatTraversalAgent;
      }

      void handleRemoteNATedToRemoteNATedCall()
      {
         const char* message =
            "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
            "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: 297\r\n"
            "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=47.135.162.145;rport=14956\r\n"
            "\r\n"
            "\r\n"
            "v=0\r\n"
            "o=LGEIPP 874 874 IN IP4 192.168.1.101\r\n"
            "s=SIP Call\r\n"
            "c=IN IP4 192.168.1.101\r\n"
            "t=0 0\r\n"
            "m=audio 23000 RTP/AVP 18 0 8 4 101\r\n"
            "a=rtpmap:18 G729/8000\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
            "a=rtpmap:8 PCMA/8000\r\n"
            "a=rtpmap:4 G723/8000\r\n"
            "a=rtpmap:101 telephone-event/8000\r\n"
            "a=fmtp:18 annexb=no\r\n"
            "a=fmtp:101 0-11\r\n"
            "a=sendrecv\r\n";
         SipMessage testMsg(message, strlen(message));
         testMsg.setSendProtocol(OsSocket::UDP);

         UtlString rejectReason;
         UtlSList emptyList;
         RouteState routeState( message, emptyList, "192.168.0.2:5060" );

         AuthPlugin::AuthResult result;
         result = pNatTraversalAgent->authorizeAndModify( "id",                             //dummy
                                                          Url("sip:601@192.168.1.11:5060"), //dummy
                                                          routeState,
                                                          "INVITE",
                                                          AuthPlugin::CONTINUE,
                                                          testMsg,
                                                          false,
                                                          rejectReason );

         // check auth result
         CPPUNIT_ASSERT( result == AuthPlugin::CONTINUE );

         // check that call tracker got created
         UtlString callId;
         CallTracker* pTracker;
         testMsg.getCallIdField( &callId );
         pTracker = pNatTraversalAgent->getCallTrackerFromCallId( callId );
         CPPUNIT_ASSERT( pTracker != 0 );

         // check that public transport of caller got encoded in route state
         UtlString transportInfo;
         routeState.getParameter( pNatTraversalAgent->mInstanceName.data(), CALLER_PUBLIC_TRANSPORT_PARAM, transportInfo );
         ASSERT_STR_EQUAL( "47.135.162.145:14956;transport=udp", transportInfo.data()  );

         // check that public transport of callee got encoded in route state
         routeState.getParameter( pNatTraversalAgent->mInstanceName.data(), CALLEE_PUBLIC_TRANSPORT_PARAM, transportInfo );
         ASSERT_STR_EQUAL( "47.135.162.145:29544;transport=udp", transportInfo.data()  );

         // Check that sipX NAT route is set to reach the callee's public IP address
         UtlString natRoute;
         CPPUNIT_ASSERT( testMsg.getSipXNatRoute( &natRoute ) == true );
         ASSERT_STR_EQUAL( "47.135.162.145:29544;transport=udp", natRoute.data() );
         // Now present the message to the Output Processor and verify that it got processed correctly.
         routeState.update( &testMsg ); // uodate route state as it would normally be done by SipRouter.
         pNatTraversalAgent->handleOutputMessage( testMsg, "47.135.162.145", 29544 );

         // verify that top via got adjusted to reflect our public IP address
         UtlString topmostVia;
         CPPUNIT_ASSERT( testMsg.getViaFieldSubField( &topmostVia, 0 ) );
         ASSERT_STR_EQUAL( "SIP/2.0/UDP 47.135.162.140:5060;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a",
                           topmostVia.data() );

         // verify that top route got modfified to reflect our public address
         UtlString tmpRecordRoute;
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri( 0, &tmpRecordRoute ) );
         Url tmpRecordRouteUrl( tmpRecordRoute );
         UtlString tmpRecordRouteHost;
         tmpRecordRouteUrl.getHostWithPort( tmpRecordRouteHost );
         ASSERT_STR_EQUAL( "47.135.162.140:5060", tmpRecordRouteHost.data() );

         // recall the call tracker based on message and compare with previous
         CallTracker* pTracker2;
         pTracker2 = pNatTraversalAgent->getCallTrackerForMessage( testMsg );
         CPPUNIT_ASSERT( pTracker2 == pTracker );

         // check that the SDP got changed properly to relay media
         const SdpBody* pSdpBody = testMsg.getSdpBody();
         CPPUNIT_ASSERT( pSdpBody );
         UtlString mediaType, mediaTransportType;
         int mediaPort, mediaPortPairs, numPayloadTypes, payloadTypes[10];
         SdpDirectionality dir;

         CPPUNIT_ASSERT( pSdpBody->getMediaData( 0,
                                 &mediaType,
                                 &mediaPort,
                                 &mediaPortPairs,
                                 &mediaTransportType,
                                 10,
                                 &numPayloadTypes,
                                 payloadTypes,
                                 &dir ) == TRUE );
         CPPUNIT_ASSERT( mediaType == "audio" );
         CPPUNIT_ASSERT( mediaPort == 10000 );
         CPPUNIT_ASSERT( mediaPortPairs == 1 );
         CPPUNIT_ASSERT( mediaTransportType == "RTP/AVP" );
         CPPUNIT_ASSERT( numPayloadTypes == 5 );
         CPPUNIT_ASSERT( payloadTypes[0] == 18 );
         CPPUNIT_ASSERT( payloadTypes[1] == 0 );
         CPPUNIT_ASSERT( payloadTypes[2] == 8 );
         CPPUNIT_ASSERT( payloadTypes[3] == 4 );
         CPPUNIT_ASSERT( payloadTypes[4] == 101 );
         CPPUNIT_ASSERT( dir == sdpDirectionalitySendRecv );

         // check that we are using the public IP address of the media relay
         UtlString mediaAddress;
         CPPUNIT_ASSERT( pSdpBody->getMediaAddress(0, &mediaAddress ) );
         ASSERT_STR_EQUAL( pNatTraversalAgent->mNatTraversalRules.getMediaRelayPublicAddress(), mediaAddress );

         // Mow simulalate the reception of the response
         const char* response =
         "SIP/2.0 200 OK\r\n"
         "From: \"R1_2 - 602\"<sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=94bf84d0-c0a8010b-13c4-3e8d1-496994c2-3e8d1\r\n"
         "Call-ID: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "CSeq: 1 INVITE\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;received=47.135.162.145;rport=14956;branch=z9hG4bK-3e635-f3b41fc-310ddca7;id=1234-0\r\n"
         "Supported: replaces\r\n"
         "Contact: <sip:601@192.168.1.11:5060>\r\n"
         "Record-Route: <sip:47.135.162.140:5060;lr;sipXecs-rs=%2Afrom%7EOTRiYzI1YjgtYzBhODAxNjUtMTNjNC0zZTYzNS0zN2FhMTk4OS0zZTYzNQ%60%60.400_authrules%2Aauth%7E.900_ntap%2ACeT%7ENDcuMTM1LjE2Mi4xNDU6Mjk1NDQ7dHJhbnNwb3J0PXVkcA%60%60.900_ntap%2ACrT%7ENDcuMTM1LjE2Mi4xNDU6MTQ5NTY7dHJhbnNwb3J0PXVkcA%60%60.900_ntap%2Aid%7EMC0w%215d4c53f8d9c9ecd8d089e57ba7a59513>\r\n"
         "Content-Type: application/sdp\r\n"
         "Content-Length: 202\r\n"
         "\r\n"
         "\r\n"
         "v=0\r\n"
         "o=LGEIPP 5466 5466 IN IP4 192.168.1.11\r\n"
         "s=SIP Call\r\n"
         "c=IN IP4 192.168.1.11\r\n"
         "t=0 0\r\n"
         "m=audio 23020 RTP/AVP 0 101\r\n"
         "a=rtpmap:0 PCMU/8000\r\n"
         "a=rtpmap:101 telephone-event/8000\r\n"
         "a=fmtp:101 0-11\r\n"
         "a=sendrecv\r\n"
         "a=x-sipx-ntap:192.168.0.2;1\r\n";
         SipMessage okResp( response, strlen(response) );
         okResp.setSendProtocol(OsSocket::UDP);
         
         pNatTraversalAgent->handleOutputMessage( okResp, "47.135.162.145", 14956 );

         // check that the record route still has our public IP address
         CPPUNIT_ASSERT( okResp.getRecordRouteUri( 0, &tmpRecordRoute ) );
         Url tmpRecordRouteUrl2( tmpRecordRoute );
         tmpRecordRouteUrl2.getHostWithPort( tmpRecordRouteHost );
         ASSERT_STR_EQUAL( "47.135.162.140:5060", tmpRecordRouteHost.data() );

         // check that the SDP got changed properly to relay media
         pSdpBody = okResp.getSdpBody();
         CPPUNIT_ASSERT( pSdpBody );
         CPPUNIT_ASSERT( pSdpBody->getMediaData( 0,
                                 &mediaType,
                                 &mediaPort,
                                 &mediaPortPairs,
                                 &mediaTransportType,
                                 10,
                                 &numPayloadTypes,
                                 payloadTypes,
                                 &dir ) == TRUE );
         CPPUNIT_ASSERT( mediaType == "audio" );
         CPPUNIT_ASSERT( mediaPort == 11000 );
         CPPUNIT_ASSERT( mediaPortPairs == 1 );
         CPPUNIT_ASSERT( mediaTransportType == "RTP/AVP" );
         CPPUNIT_ASSERT( numPayloadTypes == 2 );
         CPPUNIT_ASSERT( payloadTypes[0] == 0);
         CPPUNIT_ASSERT( payloadTypes[1] == 101 );
         CPPUNIT_ASSERT( dir == sdpDirectionalitySendRecv );

         // check that we are using the public IP address of the media relay
         CPPUNIT_ASSERT( pSdpBody->getMediaAddress(0, &mediaAddress ) );
         ASSERT_STR_EQUAL( pNatTraversalAgent->mNatTraversalRules.getMediaRelayPublicAddress(), mediaAddress );
      }

      void handleLocalNATedToRemoteNATedCall()
      {
         const char* message =
           "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
           "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
           "From: \"L3\"<sip:403@rjolyscs2.ca.nortel.com>;tag=94bb9480-c0a80065-13c4-50045-20c55bd3-50045\r\n"
           "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
           "Call-Id: 94bb55a0-c0a80065-13c4-50045-61a3ad3-50045@rjolyscs2.ca.nortel.com\r\n"
           "Cseq: 1 INVITE\r\n"
           "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-8130adee90e0277761fec4ee1c194a56838f~9d6590763d78764b93a42e94dbd6b75a\r\n"
           "Via: SIP/2.0/UDP 192.168.0.101:5060;branch=z9hG4bK-50045-13890e4e-4f17a229\r\n"
           "Max-Forwards: 19\r\n"
           "Supported: replaces\r\n"
           "User-Agent: LG-Nortel LIP 6812 v1.2.38sp SN/00405A183385\r\n"
           "Contact: <sip:403@192.168.0.101:5060;x-sipX-nonat>\r\n"
           "Content-Type: application/sdp\r\n"
           "Content-Length: 301\r\n"
           "Date: Fri, 13 Jun 2008 14:48:44 GMT\r\n"
           "\r\n"
           "\r\n"
           "v=0\r\n"
           "o=LGEIPP 14595 14595 IN IP4 192.168.0.101\r\n"
           "s=SIP Call\r\n"
           "c=IN IP4 192.168.0.101\r\n"
           "t=0 0\r\n"
           "m=audio 23016 RTP/AVP 18 4 0 8 101\r\n"
           "a=rtpmap:18 G729/8000\r\n"
           "a=rtpmap:4 G723/8000\r\n"
           "a=rtpmap:0 PCMU/8000\r\n"
           "a=rtpmap:8 PCMA/8000\r\n"
           "a=rtpmap:101 telephone-event/8000\r\n"
           "a=fmtp:18 annexb=no\r\n"
           "a=fmtp:101 0-11\r\n"
           "a=sendrecv\r\n";
         SipMessage testMsg(message, strlen(message));
         testMsg.setSendProtocol(OsSocket::UDP);
         
         UtlString rejectReason;
         UtlSList emptyList;
         RouteState routeState( message, emptyList, "192.168.0.2:5060" );

         AuthPlugin::AuthResult result;
         result = pNatTraversalAgent->authorizeAndModify( "id",                             //dummy
                                                          Url("sip:601@192.168.1.11:5060"), //dummy
                                                          routeState,
                                                          "INVITE",
                                                          AuthPlugin::CONTINUE,
                                                          testMsg,
                                                          false,
                                                          rejectReason );

         // check auth result
         CPPUNIT_ASSERT( result == AuthPlugin::CONTINUE );

         // check that call tracker got created
         UtlString callId;
         CallTracker* pTracker;
         testMsg.getCallIdField( &callId );
         pTracker = pNatTraversalAgent->getCallTrackerFromCallId( callId );
         CPPUNIT_ASSERT( pTracker != 0 );

         // check that no public transport of caller got encoded in route state
         UtlString transportInfo;
         CPPUNIT_ASSERT( !routeState.getParameter( pNatTraversalAgent->mInstanceName.data(), CALLER_PUBLIC_TRANSPORT_PARAM, transportInfo ) );

         // check that public transport of callee got encoded in route state
         routeState.getParameter( pNatTraversalAgent->mInstanceName.data(), CALLEE_PUBLIC_TRANSPORT_PARAM, transportInfo );
         ASSERT_STR_EQUAL( "47.135.162.145:29544;transport=udp", transportInfo.data()  );

         // Check that sipX NAT route is set to reach the callee's public IP address
         UtlString natRoute;
         CPPUNIT_ASSERT( testMsg.getSipXNatRoute( &natRoute ) == true );
         ASSERT_STR_EQUAL( "47.135.162.145:29544;transport=udp", natRoute.data() );
         // Now present the message to the Output Processor and verify that it got processed correctly.
         routeState.update( &testMsg ); // uodate route state as it would normally be done by SipRouter.
         pNatTraversalAgent->handleOutputMessage( testMsg, "47.135.162.145", 29544 );

         // verify that top via got adjusted to reflect our public IP address
         UtlString topmostVia;
         CPPUNIT_ASSERT( testMsg.getViaFieldSubField( &topmostVia, 0 ) );
         ASSERT_STR_EQUAL( "SIP/2.0/UDP 47.135.162.140:5060;branch=z9hG4bK-sipXecs-8130adee90e0277761fec4ee1c194a56838f~9d6590763d78764b93a42e94dbd6b75a",
                           topmostVia.data() );

         // verify that top route got modfified to reflect our public address
         UtlString tmpRecordRoute;
         CPPUNIT_ASSERT( testMsg.getRecordRouteUri( 0, &tmpRecordRoute ) );
         Url tmpRecordRouteUrl( tmpRecordRoute );
         UtlString tmpRecordRouteHost;
         tmpRecordRouteUrl.getHostWithPort( tmpRecordRouteHost );
         ASSERT_STR_EQUAL( "47.135.162.140:5060", tmpRecordRouteHost.data() );

         // recall the call tracker based on message and compare with previous
         CallTracker* pTracker2;
         pTracker2 = pNatTraversalAgent->getCallTrackerForMessage( testMsg );
         CPPUNIT_ASSERT( pTracker2 == pTracker );

         // check that the SDP got changed properly to relay media
         const SdpBody* pSdpBody = testMsg.getSdpBody();
         CPPUNIT_ASSERT( pSdpBody );
         UtlString mediaType, mediaTransportType;
         int mediaPort, mediaPortPairs, numPayloadTypes, payloadTypes[10];
         SdpDirectionality dir;

         CPPUNIT_ASSERT( pSdpBody->getMediaData( 0,
                                 &mediaType,
                                 &mediaPort,
                                 &mediaPortPairs,
                                 &mediaTransportType,
                                 10,
                                 &numPayloadTypes,
                                 payloadTypes,
                                 &dir ) == TRUE );
         CPPUNIT_ASSERT( mediaType == "audio" );
         CPPUNIT_ASSERT( mediaPort == 10000 );
         CPPUNIT_ASSERT( mediaPortPairs == 1 );
         CPPUNIT_ASSERT( mediaTransportType == "RTP/AVP" );
         CPPUNIT_ASSERT( numPayloadTypes == 5 );
         CPPUNIT_ASSERT( payloadTypes[0] == 18 );
         CPPUNIT_ASSERT( payloadTypes[1] == 4 );
         CPPUNIT_ASSERT( payloadTypes[2] == 0 );
         CPPUNIT_ASSERT( payloadTypes[3] == 8 );
         CPPUNIT_ASSERT( payloadTypes[4] == 101 );
         CPPUNIT_ASSERT( dir == sdpDirectionalitySendRecv );

         // check that we are using the public IP address of the media relay
         UtlString mediaAddress;
         CPPUNIT_ASSERT( pSdpBody->getMediaAddress(0, &mediaAddress ) );
         ASSERT_STR_EQUAL( pNatTraversalAgent->mNatTraversalRules.getMediaRelayPublicAddress(), mediaAddress );

         // Mow simulalate the reception of the response
         const char* response =
         "SIP/2.0 200 OK\r\n"
         "From: \"L3\"<sip:403@rjolyscs2.ca.nortel.com>;tag=94bb9480-c0a80065-13c4-50045-20c55bd3-50045\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=94bf81c0-c0a8010b-13c4-4fc07-650a9211-4fc07\r\n"
         "Call-Id: 94bb55a0-c0a80065-13c4-50045-61a3ad3-50045@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Via: SIP/2.0/UDP 192.168.0.101:5060;branch=z9hG4bK-50045-13890e4e-4f17a229;id=1234-1\r\n"
         "Supported: replaces\r\n"
         "Contact: <sip:601@192.168.1.11:5060>\r\n"
         "Record-Route: <sip:47.135.162.140:5060;lr;sipXecs-rs=%2Afrom%7EOTRiYjk0ODAtYzBhODAwNjUtMTNjNC01MDA0NS0yMGM1NWJkMy01MDA0NQ%60%60.400_authrules%2Aauth%7E.900_ntap%2ACeT%7ENDcuMTM1LjE2Mi4xNDU6Mjk1NDQ7dHJhbnNwb3J0PXVkcA%60%60.900_ntap%2Aid%7EMy0z%21641c6aa1807d3544000f310a64a5360a>\r\n"
         "Content-Type: application/sdp\r\n"
         "Content-Length: 250\r\n"
         "Date: Fri, 13 Jun 2008 14:48:46 GMT\r\n"
         "\r\n"
         "\r\n"
         "v=0\r\n"
         "o=LGEIPP 12617 12617 IN IP4 192.168.1.11\r\n"
         "s=SIP Call\r\n"
         "c=IN IP4 192.168.1.11\r\n"
         "t=0 0\r\n"
         "m=audio 25038 RTP/AVP 4 101\r\n"
         "c=IN IP4 192.168.0.2\r\n"
         "a=rtpmap:4 G723/8000\r\n"
         "a=rtpmap:101 telephone-event/8000\r\n"
         "a=fmtp:101 0-11\r\n"
         "a=sendrecv\r\n"
         "a=x-sipx-ntap:192.168.0.2;3\r\n";
         SipMessage okResp( response, strlen(response) );
         okResp.setSendProtocol(OsSocket::UDP);
         
         pNatTraversalAgent->handleOutputMessage( okResp, "192.168.0.101", 5060 );

         // check that the record route  has our private IP address
         CPPUNIT_ASSERT( okResp.getRecordRouteUri( 0, &tmpRecordRoute ) );
         Url tmpRecordRouteUrl2( tmpRecordRoute );
         tmpRecordRouteUrl2.getHostWithPort( tmpRecordRouteHost );
         ASSERT_STR_EQUAL( "192.168.0.2:5060", tmpRecordRouteHost.data() );

         // check that the SDP got changed properly to relay media
         pSdpBody = okResp.getSdpBody();
         CPPUNIT_ASSERT( pSdpBody );
         CPPUNIT_ASSERT( pSdpBody->getMediaData( 0,
                                 &mediaType,
                                 &mediaPort,
                                 &mediaPortPairs,
                                 &mediaTransportType,
                                 10,
                                 &numPayloadTypes,
                                 payloadTypes,
                                 &dir ) == TRUE );
         CPPUNIT_ASSERT( mediaType == "audio" );
         CPPUNIT_ASSERT( mediaPort == 11000 );
         CPPUNIT_ASSERT( mediaPortPairs == 1 );
         CPPUNIT_ASSERT( mediaTransportType == "RTP/AVP" );
         CPPUNIT_ASSERT( numPayloadTypes == 2 );
         CPPUNIT_ASSERT( payloadTypes[0] == 4);
         CPPUNIT_ASSERT( payloadTypes[1] == 101 );
         CPPUNIT_ASSERT( dir == sdpDirectionalitySendRecv );

         // check that we are using the native IP address of the media relay
         CPPUNIT_ASSERT( pSdpBody->getMediaAddress(0, &mediaAddress ) );
         ASSERT_STR_EQUAL( pNatTraversalAgent->mNatTraversalRules.getMediaRelayNativeAddress(), mediaAddress );
      }

      void stalledMediaStreamDetectionTest_StreamNotStalled()
      {
         SetFakeReceptionOfPacketsFlag( true );
         RouteState::setSecret("fixed");

         const char* message =
           "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
           "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
           "From: \"L3\"<sip:403@rjolyscs2.ca.nortel.com>;tag=94bb9480-c0a80065-13c4-50045-20c55bd3-50045\r\n"
           "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
           "Call-Id: 94bb55a0-c0a80065-13c4-50045-61a3ad3-50045@rjolyscs2.ca.nortel.com\r\n"
           "Cseq: 1 INVITE\r\n"
           "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-8130adee90e0277761fec4ee1c194a56838f~9d6590763d78764b93a42e94dbd6b75a\r\n"
           "Via: SIP/2.0/UDP 192.168.0.101:5060;branch=z9hG4bK-50045-13890e4e-4f17a229\r\n"
           "Max-Forwards: 19\r\n"
           "Supported: replaces\r\n"
           "User-Agent: LG-Nortel LIP 6812 v1.2.38sp SN/00405A183385\r\n"
           "Contact: <sip:403@192.168.0.101:5060;x-sipX-nonat>\r\n"
           "Content-Type: application/sdp\r\n"
           "Content-Length: 301\r\n"
           "Date: Fri, 13 Jun 2008 14:48:44 GMT\r\n"
           "\r\n"
           "\r\n"
           "v=0\r\n"
           "o=LGEIPP 14595 14595 IN IP4 192.168.0.101\r\n"
           "s=SIP Call\r\n"
           "c=IN IP4 192.168.0.101\r\n"
           "t=0 0\r\n"
           "m=audio 23016 RTP/AVP 18 4 0 8 101\r\n"
           "a=rtpmap:18 G729/8000\r\n"
           "a=rtpmap:4 G723/8000\r\n"
           "a=rtpmap:0 PCMU/8000\r\n"
           "a=rtpmap:8 PCMA/8000\r\n"
           "a=rtpmap:101 telephone-event/8000\r\n"
           "a=fmtp:18 annexb=no\r\n"
           "a=fmtp:101 0-11\r\n"
           "a=sendrecv\r\n";
         SipMessage testMsg(message, strlen(message));
         testMsg.setSendProtocol(OsSocket::UDP);
         
         UtlString rejectReason;
         UtlSList emptyList;
         RouteState routeState( message, emptyList, "192.168.0.2:5060" );

         AuthPlugin::AuthResult result;
         result = pNatTraversalAgent->authorizeAndModify( "id",                             //dummy
                                                          Url("sip:601@192.168.1.11:5060"), //dummy
                                                          routeState,
                                                          "INVITE",
                                                          AuthPlugin::CONTINUE,
                                                          testMsg,
                                                          false,
                                                          rejectReason );

         // check auth result
         CPPUNIT_ASSERT( result == AuthPlugin::CONTINUE );

         routeState.update( &testMsg );
         pNatTraversalAgent->handleOutputMessage( testMsg, "47.135.162.145", 29544 );

         // Mow simulalate the reception of the response
         const char* response =
         "SIP/2.0 200 OK\r\n"
         "From: \"L3\"<sip:403@rjolyscs2.ca.nortel.com>;tag=94bb9480-c0a80065-13c4-50045-20c55bd3-50045\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=94bf81c0-c0a8010b-13c4-4fc07-650a9211-4fc07\r\n"
         "Call-Id: 94bb55a0-c0a80065-13c4-50045-61a3ad3-50045@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Via: SIP/2.0/UDP 192.168.0.101:5060;branch=z9hG4bK-50045-13890e4e-4f17a229;id=1234-2\r\n"
         "Supported: replaces\r\n"
         "Contact: <sip:601@192.168.1.11:5060>\r\n"
         "Record-Route: <sip:47.135.162.140:5060;lr;sipXecs-rs=%2Afrom%7EOTRiYjk0ODAtYzBhODAwNjUtMTNjNC01MDA0NS0yMGM1NWJkMy01MDA0NQ%60%60.ntap%2ACeT%7ENDcuMTM1LjE2Mi4xNDU6Mjk1NDQ7dHJhbnNwb3J0PXVkcA%60%60.ntap%2Aid%7EMTIzNC0y%21370cba24fde84e23f15b8e1dc8f3633a>\r\n"
         "Content-Type: application/sdp\r\n"
         "Content-Length: 250\r\n"
         "Date: Fri, 13 Jun 2008 14:48:46 GMT\r\n"
         "\r\n"
         "\r\n"
         "v=0\r\n"
         "o=LGEIPP 12617 12617 IN IP4 192.168.1.11\r\n"
         "s=SIP Call\r\n"
         "c=IN IP4 192.168.1.11\r\n"
         "t=0 0\r\n"
         "m=audio 25038 RTP/AVP 4 101\r\n"
         "c=IN IP4 192.168.0.2\r\n"
         "a=rtpmap:4 G723/8000\r\n"
         "a=rtpmap:101 telephone-event/8000\r\n"
         "a=fmtp:101 0-11\r\n"
         "a=sendrecv\r\n"
         "a=x-sipx-ntap:192.168.0.2;3\r\n";
         SipMessage okResp( response, strlen(response) );
         okResp.setSendProtocol(OsSocket::UDP);
         
         pNatTraversalAgent->handleOutputMessage( okResp, "192.168.0.101", 5060 );

         // Mow simulalate the reception of the ACK
         const char* ack =
            "ACK sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "From: \"L3\"<sip:403@rjolyscs2.ca.nortel.com>;tag=94bb9480-c0a80065-13c4-50045-20c55bd3-50045\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=94bf81c0-c0a8010b-13c4-4fc07-650a9211-4fc07\r\n"
            "Call-Id: 94bb55a0-c0a80065-13c4-50045-61a3ad3-50045@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 ACK\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-8130adee90e0277761fec4ee1c194a56838f~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "Via: SIP/2.0/UDP 192.168.0.101:5060;branch=z9hG4bK-50045-13890e4e-4f17a229\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "User-Agent: LG-Nortel LIP 6812 v1.2.38sp SN/00405A183385\r\n"
            "Contact: <sip:403@192.168.0.101:5060;x-sipX-nonat>\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
            SipMessage ackRequest( ack, strlen(ack) );
         ackRequest.setSendProtocol(OsSocket::UDP);
         
         UtlSList routeList;
         UtlString poppedRoute("<sip:47.135.162.140:5060;lr;sipXecs-rs=%2Afrom%7EOTRiYjk0ODAtYzBhODAwNjUtMTNjNC01MDA0NS0yMGM1NWJkMy01MDA0NQ%60%60.ntap%2ACeT%7ENDcuMTM1LjE2Mi4xNDU6Mjk1NDQ7dHJhbnNwb3J0PXVkcA%60%60.ntap%2Aid%7EMTIzNC0y%21370cba24fde84e23f15b8e1dc8f3633a>");
         routeList.append( &poppedRoute );
         RouteState newRouteState( message, routeList, "192.168.0.2:5060" );

         result = pNatTraversalAgent->authorizeAndModify( "id",                             //dummy
                                                          Url("sip:601@192.168.1.11:5060"), //dummy
                                                          newRouteState,
                                                          "ACK",
                                                          AuthPlugin::CONTINUE,
                                                          ackRequest,
                                                          false,
                                                          rejectReason );

         // check auth result
         CPPUNIT_ASSERT( result == AuthPlugin::CONTINUE );
         pNatTraversalAgent->handleOutputMessage( ackRequest, "47.135.162.145", 29544 );

         // Check that the DialogTracker is still around after 10 seconds.
         CallTracker* pTracker;
         UtlString callId;
         testMsg.getCallIdField( &callId );
         pTracker = pNatTraversalAgent->getCallTrackerFromCallId( callId );
         CPPUNIT_ASSERT( pTracker != 0 );
         CPPUNIT_ASSERT( pTracker->mSessionContextsMap.entries() == 1 );
         sleep( MAX_TIMER_TICK_COUNTS_BEFORE_CALL_TRACKER_CLEAN_UP * CLEAN_UP_TIMER_IN_SECS * 2 /*safety margin*/ );
         testMsg.getCallIdField( &callId );
         pTracker = pNatTraversalAgent->getCallTrackerFromCallId( callId );
         CPPUNIT_ASSERT( pTracker != 0 );
         CPPUNIT_ASSERT( pTracker->mSessionContextsMap.entries() == 1 );
      }

      void stalledMediaStreamDetectionTest_StreamStalled()
      {
         SetFakeReceptionOfPacketsFlag( false );
         RouteState::setSecret("fixed");

         const char* message =
           "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
           "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
           "From: \"L3\"<sip:403@rjolyscs2.ca.nortel.com>;tag=94bb9480-c0a80065-13c4-50045-20c55bd3-50045\r\n"
           "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
           "Call-Id: 94bb55a0-c0a80065-13c4-50045-61a3ad3-50045@rjolyscs2.ca.nortel.com\r\n"
           "Cseq: 1 INVITE\r\n"
           "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-8130adee90e0277761fec4ee1c194a56838f~9d6590763d78764b93a42e94dbd6b75a\r\n"
           "Via: SIP/2.0/UDP 192.168.0.101:5060;branch=z9hG4bK-50045-13890e4e-4f17a229\r\n"
           "Max-Forwards: 19\r\n"
           "Supported: replaces\r\n"
           "User-Agent: LG-Nortel LIP 6812 v1.2.38sp SN/00405A183385\r\n"
           "Contact: <sip:403@192.168.0.101:5060;x-sipX-nonat>\r\n"
           "Content-Type: application/sdp\r\n"
           "Content-Length: 301\r\n"
           "Date: Fri, 13 Jun 2008 14:48:44 GMT\r\n"
           "\r\n"
           "\r\n"
           "v=0\r\n"
           "o=LGEIPP 14595 14595 IN IP4 192.168.0.101\r\n"
           "s=SIP Call\r\n"
           "c=IN IP4 192.168.0.101\r\n"
           "t=0 0\r\n"
           "m=audio 23016 RTP/AVP 18 4 0 8 101\r\n"
           "a=rtpmap:18 G729/8000\r\n"
           "a=rtpmap:4 G723/8000\r\n"
           "a=rtpmap:0 PCMU/8000\r\n"
           "a=rtpmap:8 PCMA/8000\r\n"
           "a=rtpmap:101 telephone-event/8000\r\n"
           "a=fmtp:18 annexb=no\r\n"
           "a=fmtp:101 0-11\r\n"
           "a=sendrecv\r\n";
         SipMessage testMsg(message, strlen(message));
         testMsg.setSendProtocol(OsSocket::UDP);
         
         UtlString rejectReason;
         UtlSList emptyList;
         RouteState routeState( message, emptyList, "192.168.0.2:5060" );

         AuthPlugin::AuthResult result;
         result = pNatTraversalAgent->authorizeAndModify( "id",                             //dummy
                                                          Url("sip:601@192.168.1.11:5060"), //dummy
                                                          routeState,
                                                          "INVITE",
                                                          AuthPlugin::CONTINUE,
                                                          testMsg,
                                                          false,
                                                          rejectReason );

         // check auth result
         CPPUNIT_ASSERT( result == AuthPlugin::CONTINUE );

         routeState.update( &testMsg );
         pNatTraversalAgent->handleOutputMessage( testMsg, "47.135.162.145", 29544 );

         // Mow simulalate the reception of the response
         const char* response =
         "SIP/2.0 200 OK\r\n"
         "From: \"L3\"<sip:403@rjolyscs2.ca.nortel.com>;tag=94bb9480-c0a80065-13c4-50045-20c55bd3-50045\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=94bf81c0-c0a8010b-13c4-4fc07-650a9211-4fc07\r\n"
         "Call-Id: 94bb55a0-c0a80065-13c4-50045-61a3ad3-50045@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Via: SIP/2.0/UDP 192.168.0.101:5060;branch=z9hG4bK-50045-13890e4e-4f17a229;id=1234-3\r\n"
         "Supported: replaces\r\n"
         "Contact: <sip:601@192.168.1.11:5060>\r\n"
         "Record-Route: <sip:47.135.162.140:5060;lr;sipXecs-rs=%2Afrom%7EOTRiYjk0ODAtYzBhODAwNjUtMTNjNC01MDA0NS0yMGM1NWJkMy01MDA0NQ%60%60.ntap%2ACeT%7ENDcuMTM1LjE2Mi4xNDU6Mjk1NDQ7dHJhbnNwb3J0PXVkcA%60%60.ntap%2Aid%7EMTIzNC0z%213d7b53562948991d1cf59dfe7fdaaf24>\r\n"
         "Content-Type: application/sdp\r\n"
         "Content-Length: 250\r\n"
         "Date: Fri, 13 Jun 2008 14:48:46 GMT\r\n"
         "\r\n"
         "\r\n"
         "v=0\r\n"
         "o=LGEIPP 12617 12617 IN IP4 192.168.1.11\r\n"
         "s=SIP Call\r\n"
         "c=IN IP4 192.168.1.11\r\n"
         "t=0 0\r\n"
         "m=audio 25038 RTP/AVP 4 101\r\n"
         "c=IN IP4 192.168.0.2\r\n"
         "a=rtpmap:4 G723/8000\r\n"
         "a=rtpmap:101 telephone-event/8000\r\n"
         "a=fmtp:101 0-11\r\n"
         "a=sendrecv\r\n"
         "a=x-sipx-ntap:192.168.0.2;3\r\n";
         SipMessage okResp( response, strlen(response) );
         okResp.setSendProtocol(OsSocket::UDP);
         
         pNatTraversalAgent->handleOutputMessage( okResp, "192.168.0.101", 5060 );

         // Mow simulalate the reception of the ACK
         const char* ack =
            "ACK sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "From: \"L3\"<sip:403@rjolyscs2.ca.nortel.com>;tag=94bb9480-c0a80065-13c4-50045-20c55bd3-50045\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=94bf81c0-c0a8010b-13c4-4fc07-650a9211-4fc07\r\n"
            "Call-Id: 94bb55a0-c0a80065-13c4-50045-61a3ad3-50045@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 ACK\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-8130adee90e0277761fec4ee1c194a56838f~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "Via: SIP/2.0/UDP 192.168.0.101:5060;branch=z9hG4bK-50045-13890e4e-4f17a229\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "User-Agent: LG-Nortel LIP 6812 v1.2.38sp SN/00405A183385\r\n"
            "Contact: <sip:403@192.168.0.101:5060;x-sipX-nonat>\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
            SipMessage ackRequest( ack, strlen(ack) );
         ackRequest.setSendProtocol(OsSocket::UDP);
         
         UtlSList routeList;
         UtlString poppedRoute("<sip:47.135.162.140:5060;lr;sipXecs-rs=%2Afrom%7EOTRiYjk0ODAtYzBhODAwNjUtMTNjNC01MDA0NS0yMGM1NWJkMy01MDA0NQ%60%60.ntap%2ACeT%7ENDcuMTM1LjE2Mi4xNDU6Mjk1NDQ7dHJhbnNwb3J0PXVkcA%60%60.ntap%2Aid%7EMTIzNC0z%213d7b53562948991d1cf59dfe7fdaaf24>");
         routeList.append( &poppedRoute );
         RouteState newRouteState( message, routeList, "192.168.0.2:5060" );

         result = pNatTraversalAgent->authorizeAndModify( "id",                             //dummy
                                                          Url("sip:601@192.168.1.11:5060"), //dummy
                                                          newRouteState,
                                                          "ACK",
                                                          AuthPlugin::CONTINUE,
                                                          ackRequest,
                                                          false,
                                                          rejectReason );

         // check auth result
         CPPUNIT_ASSERT( result == AuthPlugin::CONTINUE );
         pNatTraversalAgent->handleOutputMessage( ackRequest, "47.135.162.145", 29544 );

         // Check that the DialogTracker is still around after 10 seconds.
         CallTracker* pTracker;
         UtlString callId;
         testMsg.getCallIdField( &callId );
         pTracker = pNatTraversalAgent->getCallTrackerFromCallId( callId );
         CPPUNIT_ASSERT( pTracker != 0 );
         CPPUNIT_ASSERT( pTracker->mSessionContextsMap.entries() == 1 );
         sleep( MAX_TIMER_TICK_COUNTS_BEFORE_CALL_TRACKER_CLEAN_UP * CLEAN_UP_TIMER_IN_SECS * 2 /*safety margin*/ );
         pTracker = pNatTraversalAgent->getCallTrackerFromCallId( callId );
         CPPUNIT_ASSERT( pTracker == 0 );
      }

      void UndoChangesToRequestUriTests()
      {
         // with x-sipX-privcontact as only parameter
         UtlString modifiedRuri;
         const char* message1 =
           "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
           "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
           "From: \"L3\"<sip:403@rjolyscs2.ca.nortel.com>;tag=94bb9480-c0a80065-13c4-50045-20c55bd3-50045\r\n"
           "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
           "Call-Id: 94bb55a0-c0a80065-13c4-50045-61a3ad3-50045@rjolyscs2.ca.nortel.com\r\n"
           "Cseq: 1 INVITE\r\n"
           "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-8130adee90e0277761fec4ee1c194a56838f~9d6590763d78764b93a42e94dbd6b75a\r\n"
           "Via: SIP/2.0/UDP 192.168.0.101:5060;branch=z9hG4bK-50045-13890e4e-4f17a229\r\n"
           "Max-Forwards: 19\r\n"
           "Supported: replaces\r\n"
           "Contact: <sip:403@192.168.0.101:5060;x-sipX-nonat>\r\n"
           "Content-Length: 0\r\n"
           "\r\n";
         SipMessage testMsg1(message1, strlen(message1));
         testMsg1.setSendProtocol(OsSocket::UDP);
         
         pNatTraversalAgent->UndoChangesToRequestUri( testMsg1 );
         testMsg1.getRequestUri( &modifiedRuri );
         ASSERT_STR_EQUAL( "sip:601@192.168.1.11:5060", modifiedRuri.data() );

         // without x-sipX-privcontact
         const char* message2 =
           "INVITE sip:601@47.135.162.145:29544 SIP/2.0\r\n"
           "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
           "From: \"L3\"<sip:403@rjolyscs2.ca.nortel.com>;tag=94bb9480-c0a80065-13c4-50045-20c55bd3-50045\r\n"
           "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
           "Call-Id: 94bb55a0-c0a80065-13c4-50045-61a3ad3-50045@rjolyscs2.ca.nortel.com\r\n"
           "Cseq: 1 INVITE\r\n"
           "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-8130adee90e0277761fec4ee1c194a56838f~9d6590763d78764b93a42e94dbd6b75a\r\n"
           "Via: SIP/2.0/UDP 192.168.0.101:5060;branch=z9hG4bK-50045-13890e4e-4f17a229\r\n"
           "Max-Forwards: 19\r\n"
           "Supported: replaces\r\n"
           "Contact: <sip:403@192.168.0.101:5060;x-sipX-nonat>\r\n"
           "Content-Length: 0\r\n"
           "\r\n";
         SipMessage testMsg2(message2, strlen(message2));
         testMsg2.setSendProtocol(OsSocket::UDP);
         
         pNatTraversalAgent->UndoChangesToRequestUri( testMsg2 );
         testMsg2.getRequestUri( &modifiedRuri );
         ASSERT_STR_EQUAL( "sip:601@47.135.162.145:29544", modifiedRuri.data() );

         // with x-sipX-privcontact as first of many params
         const char* message3 =
            "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060;someparam1=12112;someparam2=12222 SIP/2.0\r\n"
           "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
           "From: \"L3\"<sip:403@rjolyscs2.ca.nortel.com>;tag=94bb9480-c0a80065-13c4-50045-20c55bd3-50045\r\n"
           "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
           "Call-Id: 94bb55a0-c0a80065-13c4-50045-61a3ad3-50045@rjolyscs2.ca.nortel.com\r\n"
           "Cseq: 1 INVITE\r\n"
           "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-8130adee90e0277761fec4ee1c194a56838f~9d6590763d78764b93a42e94dbd6b75a\r\n"
           "Via: SIP/2.0/UDP 192.168.0.101:5060;branch=z9hG4bK-50045-13890e4e-4f17a229\r\n"
           "Max-Forwards: 19\r\n"
           "Supported: replaces\r\n"
           "Contact: <sip:403@192.168.0.101:5060;x-sipX-nonat>\r\n"
           "Content-Length: 0\r\n"
           "\r\n";
         SipMessage testMsg3(message3, strlen(message3));
         testMsg3.setSendProtocol(OsSocket::UDP);
         pNatTraversalAgent->UndoChangesToRequestUri( testMsg3 );
         testMsg3.getRequestUri( &modifiedRuri );
         ASSERT_STR_EQUAL( "sip:601@192.168.1.11:5060;someparam1=12112;someparam2=12222", modifiedRuri.data() );

         // with x-sipX-privcontact as middle param
         const char* message4 =
            "INVITE sip:601@47.135.162.145:29544;someparam1=12112;x-sipX-privcontact=192.168.1.11%3A5060;someparam2=12222 SIP/2.0\r\n"
           "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
           "From: \"L3\"<sip:403@rjolyscs2.ca.nortel.com>;tag=94bb9480-c0a80065-13c4-50045-20c55bd3-50045\r\n"
           "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
           "Call-Id: 94bb55a0-c0a80065-13c4-50045-61a3ad3-50045@rjolyscs2.ca.nortel.com\r\n"
           "Cseq: 1 INVITE\r\n"
           "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-8130adee90e0277761fec4ee1c194a56838f~9d6590763d78764b93a42e94dbd6b75a\r\n"
           "Via: SIP/2.0/UDP 192.168.0.101:5060;branch=z9hG4bK-50045-13890e4e-4f17a229\r\n"
           "Max-Forwards: 19\r\n"
           "Supported: replaces\r\n"
           "Contact: <sip:403@192.168.0.101:5060;x-sipX-nonat>\r\n"
           "Content-Length: 0\r\n"
           "\r\n";
         SipMessage testMsg4(message4, strlen(message4));
         testMsg4.setSendProtocol(OsSocket::UDP);
         pNatTraversalAgent->UndoChangesToRequestUri( testMsg4 );
         testMsg4.getRequestUri( &modifiedRuri );
         ASSERT_STR_EQUAL( "sip:601@192.168.1.11:5060;someparam1=12112;someparam2=12222", modifiedRuri.data() );

         // with x-sipX-privcontact as last param
         const char* message5 =
            "INVITE sip:601@47.135.162.145:29544;someparam1=12112;someparam2=12222;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
           "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
           "From: \"L3\"<sip:403@rjolyscs2.ca.nortel.com>;tag=94bb9480-c0a80065-13c4-50045-20c55bd3-50045\r\n"
           "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
           "Call-Id: 94bb55a0-c0a80065-13c4-50045-61a3ad3-50045@rjolyscs2.ca.nortel.com\r\n"
           "Cseq: 1 INVITE\r\n"
           "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-8130adee90e0277761fec4ee1c194a56838f~9d6590763d78764b93a42e94dbd6b75a\r\n"
           "Via: SIP/2.0/UDP 192.168.0.101:5060;branch=z9hG4bK-50045-13890e4e-4f17a229\r\n"
           "Max-Forwards: 19\r\n"
           "Supported: replaces\r\n"
           "Contact: <sip:403@192.168.0.101:5060;x-sipX-nonat>\r\n"
           "Content-Length: 0\r\n"
           "\r\n";
         SipMessage testMsg5(message5, strlen(message5));
         testMsg5.setSendProtocol(OsSocket::UDP);
         pNatTraversalAgent->UndoChangesToRequestUri( testMsg5 );
         testMsg5.getRequestUri( &modifiedRuri );
         ASSERT_STR_EQUAL( "sip:601@192.168.1.11:5060;someparam1=12112;someparam2=12222", modifiedRuri.data() );

         // without x-sipX-privcontact but with other params
         const char* message6 =
            "INVITE sip:601@47.135.162.145:29544;someparam1=12112;someparam2=12222 SIP/2.0\r\n"
           "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
           "From: \"L3\"<sip:403@rjolyscs2.ca.nortel.com>;tag=94bb9480-c0a80065-13c4-50045-20c55bd3-50045\r\n"
           "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
           "Call-Id: 94bb55a0-c0a80065-13c4-50045-61a3ad3-50045@rjolyscs2.ca.nortel.com\r\n"
           "Cseq: 1 INVITE\r\n"
           "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-8130adee90e0277761fec4ee1c194a56838f~9d6590763d78764b93a42e94dbd6b75a\r\n"
           "Via: SIP/2.0/UDP 192.168.0.101:5060;branch=z9hG4bK-50045-13890e4e-4f17a229\r\n"
           "Max-Forwards: 19\r\n"
           "Supported: replaces\r\n"
           "Contact: <sip:403@192.168.0.101:5060;x-sipX-nonat>\r\n"
           "Content-Length: 0\r\n"
           "\r\n";
         SipMessage testMsg6(message6, strlen(message6));
         testMsg6.setSendProtocol(OsSocket::UDP);
         pNatTraversalAgent->UndoChangesToRequestUri( testMsg6 );
         testMsg6.getRequestUri( &modifiedRuri );
         ASSERT_STR_EQUAL( "sip:601@47.135.162.145:29544;someparam1=12112;someparam2=12222", modifiedRuri.data() );
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(NatTraversalAgentTest);
