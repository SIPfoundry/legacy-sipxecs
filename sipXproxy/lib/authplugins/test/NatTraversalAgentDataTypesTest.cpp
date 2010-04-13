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
#include "net/Url.h"
#include "NatTraversalAgentDataTypes.h"
#include "NatTraversalRules.h"

class NatTraversalDataTypesTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(NatTraversalDataTypesTest);
      // EndpointDescriptor tests
      CPPUNIT_TEST(testTransportDataSetGet);
      CPPUNIT_TEST(testEndpointDescriptor_PublicUdp);
      CPPUNIT_TEST(testEndpointDescriptor_PublicUdp_defaultPort);
      CPPUNIT_TEST(testEndpointDescriptor_PublicTcp);
      CPPUNIT_TEST(testEndpointDescriptor_RemoteNATedUdp);
      CPPUNIT_TEST(testEndpointDescriptor_RemoteNATedUdp_defaultPort);
      CPPUNIT_TEST(testEndpointDescriptor_RemoteNATedTcp);
      CPPUNIT_TEST(testEndpointDescriptor_LocalNATedUdp);
      CPPUNIT_TEST(testEndpointDescriptor_LocalNATedTcp);
      CPPUNIT_TEST(testEndpointDescriptor_UnknownUdpWithinLocalPrivateNetwork);
      CPPUNIT_TEST(testEndpointDescriptor_UnknownTcpWithinLocalPrivateNetwork);
      CPPUNIT_TEST(testEndpointDescriptor_UnknownUdpOutsideLocalPrivateNetwork);
      CPPUNIT_TEST(testEndpointDescriptor_UnknownTcpOutsideLocalPrivateNetwork);

      // MediaDescriptor tests
      CPPUNIT_TEST(testMediaDescriptor_TypeAndDirectionality);
      CPPUNIT_TEST(testMediaDescriptor_GetEndpointData);
      CPPUNIT_TEST(testMediaDescriptor_MediaRelayHandlesManip);
      CPPUNIT_TEST(testMediaDescriptor_DirectionalityConversions);

      CPPUNIT_TEST_SUITE_END();

   public:
   void testTransportDataSetGet()
   {
      TransportData transportData;

      transportData.setAddress( "47.10.10.10" );
      transportData.setPort( 6666 );
      transportData.setTransportProtocol( "tcp" );

      CPPUNIT_ASSERT( transportData.getAddress() == "47.10.10.10" );
      CPPUNIT_ASSERT( transportData.getPort() == 6666 );
      CPPUNIT_ASSERT( transportData.getTransportProtocol() == "tcp" );
   }

   void testEndpointDescriptor_PublicUdp()
   {
      NatTraversalRules natRules;
      UtlString     rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
      CPPUNIT_ASSERT( natRules.loadRules( rulesFile ) == OS_SUCCESS );

      Url publicContactUri( "<sip:501@47.135.162.161:6060;x-sipX-nonat>" );

      EndpointDescriptor endpointDescriptor( publicContactUri, natRules );

      CPPUNIT_ASSERT( endpointDescriptor.getLocationCode() == PUBLIC );


      TransportData publicTransport = endpointDescriptor.getPublicTransportAddress();
      CPPUNIT_ASSERT( publicTransport.getAddress() == "47.135.162.161" );
      CPPUNIT_ASSERT( publicTransport.getPort() == 6060 );
      CPPUNIT_ASSERT( publicTransport.getTransportProtocol() == "udp" );

      TransportData privateTransport = endpointDescriptor.getNativeTransportAddress();
      CPPUNIT_ASSERT( privateTransport.getAddress() == "47.135.162.161" );
      CPPUNIT_ASSERT( privateTransport.getPort() == 6060 );
      CPPUNIT_ASSERT( privateTransport.getTransportProtocol() == "udp" );

      CPPUNIT_ASSERT( publicTransport.isEqual( privateTransport ) );
   }

   void testEndpointDescriptor_PublicUdp_defaultPort()
   {
      NatTraversalRules natRules;
      UtlString     rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
      CPPUNIT_ASSERT( natRules.loadRules( rulesFile ) == OS_SUCCESS );

      Url publicContactUri( "<sip:501@47.135.162.161;x-sipX-nonat>" );

      EndpointDescriptor endpointDescriptor( publicContactUri, natRules );

      CPPUNIT_ASSERT( endpointDescriptor.getLocationCode() == PUBLIC );


      TransportData publicTransport = endpointDescriptor.getPublicTransportAddress();
      CPPUNIT_ASSERT( publicTransport.getAddress() == "47.135.162.161" );
      CPPUNIT_ASSERT( publicTransport.getPort() == 5060 );
      CPPUNIT_ASSERT( publicTransport.getTransportProtocol() == "udp" );

      TransportData privateTransport = endpointDescriptor.getNativeTransportAddress();
      CPPUNIT_ASSERT( privateTransport.getAddress() == "47.135.162.161" );
      CPPUNIT_ASSERT( privateTransport.getPort() == 5060 );
      CPPUNIT_ASSERT( privateTransport.getTransportProtocol() == "udp" );

      CPPUNIT_ASSERT( publicTransport.isEqual( privateTransport ) );
   }

   void testEndpointDescriptor_PublicTcp()
   {
      NatTraversalRules natRules;
      UtlString     rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
      CPPUNIT_ASSERT( natRules.loadRules( rulesFile ) == OS_SUCCESS );

      Url publicContactUri( "<sip:501@47.135.162.161:6060;transport=tcp;x-sipX-nonat>" );

      EndpointDescriptor endpointDescriptor( publicContactUri, natRules );

      CPPUNIT_ASSERT( endpointDescriptor.getLocationCode() == PUBLIC );

      TransportData publicTransport = endpointDescriptor.getPublicTransportAddress();
      CPPUNIT_ASSERT( publicTransport.getAddress() == "47.135.162.161" );
      CPPUNIT_ASSERT( publicTransport.getPort() == 6060 );
      CPPUNIT_ASSERT( publicTransport.getTransportProtocol() == "tcp" );

      TransportData privateTransport = endpointDescriptor.getNativeTransportAddress();
      CPPUNIT_ASSERT( privateTransport.getAddress() == "47.135.162.161" );
      CPPUNIT_ASSERT( privateTransport.getPort() == 6060 );
      CPPUNIT_ASSERT( privateTransport.getTransportProtocol() == "tcp" );

      CPPUNIT_ASSERT( publicTransport.isEqual( privateTransport ) );
   }

   void testEndpointDescriptor_RemoteNATedUdp()
   {
      NatTraversalRules natRules;
      UtlString     rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
      CPPUNIT_ASSERT( natRules.loadRules( rulesFile ) == OS_SUCCESS );

      Url remoteNATedContactUri( "<sip:601@47.135.162.145:10491;x-sipX-privcontact=192.168.1.11%3A6060>" );

      EndpointDescriptor endpointDescriptor( remoteNATedContactUri, natRules );

      CPPUNIT_ASSERT( endpointDescriptor.getLocationCode() == REMOTE_NATED );

      TransportData publicTransport = endpointDescriptor.getPublicTransportAddress();
      CPPUNIT_ASSERT( publicTransport.getAddress() == "47.135.162.145" );
      CPPUNIT_ASSERT( publicTransport.getPort() == 10491 );
      CPPUNIT_ASSERT( publicTransport.getTransportProtocol() == "udp" );

      TransportData privateTransport = endpointDescriptor.getNativeTransportAddress();
      CPPUNIT_ASSERT( privateTransport.getAddress() == "192.168.1.11" );
      CPPUNIT_ASSERT( privateTransport.getPort() == 6060 );
      CPPUNIT_ASSERT( privateTransport.getTransportProtocol() == "udp" );

      CPPUNIT_ASSERT( !publicTransport.isEqual( privateTransport ) );
  }

   void testEndpointDescriptor_RemoteNATedUdp_defaultPort()
   {
      NatTraversalRules natRules;
      UtlString     rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
      CPPUNIT_ASSERT( natRules.loadRules( rulesFile ) == OS_SUCCESS );

      Url remoteNATedContactUri( "<sip:601@47.135.162.145;x-sipX-privcontact=192.168.1.11>" );

      EndpointDescriptor endpointDescriptor( remoteNATedContactUri, natRules );

      CPPUNIT_ASSERT( endpointDescriptor.getLocationCode() == REMOTE_NATED );

      TransportData publicTransport = endpointDescriptor.getPublicTransportAddress();
      CPPUNIT_ASSERT( publicTransport.getAddress() == "47.135.162.145" );
      CPPUNIT_ASSERT( publicTransport.getPort() == 5060 );
      CPPUNIT_ASSERT( publicTransport.getTransportProtocol() == "udp" );

      TransportData privateTransport = endpointDescriptor.getNativeTransportAddress();
      CPPUNIT_ASSERT( privateTransport.getAddress() == "192.168.1.11" );
      CPPUNIT_ASSERT( privateTransport.getPort() == 5060 );
      CPPUNIT_ASSERT( privateTransport.getTransportProtocol() == "udp" );

      CPPUNIT_ASSERT( !publicTransport.isEqual( privateTransport ) );
   }

   void testEndpointDescriptor_RemoteNATedTcp()
   {
      NatTraversalRules natRules;
      UtlString     rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
      CPPUNIT_ASSERT( natRules.loadRules( rulesFile ) == OS_SUCCESS );

      Url remoteNATedContactUri( "<sip:601@47.135.162.145:10491;transport=tcp;x-sipX-privcontact=192.168.1.11%3A6060%3Btransport%3Dtcp>" );
      EndpointDescriptor endpointDescriptor( remoteNATedContactUri, natRules );

      CPPUNIT_ASSERT( endpointDescriptor.getLocationCode() == REMOTE_NATED );

      TransportData publicTransport = endpointDescriptor.getPublicTransportAddress();
      CPPUNIT_ASSERT( publicTransport.getAddress() == "47.135.162.145" );
      CPPUNIT_ASSERT( publicTransport.getPort() == 10491 );
      CPPUNIT_ASSERT( publicTransport.getTransportProtocol() == "tcp" );

      TransportData privateTransport = endpointDescriptor.getNativeTransportAddress();
      CPPUNIT_ASSERT( privateTransport.getAddress() == "192.168.1.11" );
      CPPUNIT_ASSERT( privateTransport.getPort() == 6060 );
      CPPUNIT_ASSERT( privateTransport.getTransportProtocol() == "tcp" );

      CPPUNIT_ASSERT( !publicTransport.isEqual( privateTransport ) );
   }

   void testEndpointDescriptor_LocalNATedUdp()
   {
      NatTraversalRules natRules;
      UtlString     rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
      CPPUNIT_ASSERT( natRules.loadRules( rulesFile ) == OS_SUCCESS );

      // pick an IP address that is part of the local topology according to
      // the NatTraversalRules prescribed by /NatTraversalAgent/nattraversalrules1.xml
      Url localNATedContactUri( "<sip:601@10.10.10.10:6060;x-sipX-nonat>" );

      EndpointDescriptor endpointDescriptor( localNATedContactUri, natRules );

      CPPUNIT_ASSERT( endpointDescriptor.getLocationCode() == LOCAL_NATED );

      TransportData publicTransport = endpointDescriptor.getPublicTransportAddress();
      CPPUNIT_ASSERT( publicTransport.getAddress() == "10.10.10.10" );
      CPPUNIT_ASSERT( publicTransport.getPort() == 6060 );
      CPPUNIT_ASSERT( publicTransport.getTransportProtocol() == "udp" );

      TransportData privateTransport = endpointDescriptor.getNativeTransportAddress();
      CPPUNIT_ASSERT( privateTransport.getAddress() == "10.10.10.10" );
      CPPUNIT_ASSERT( privateTransport.getPort() == 6060 );
      CPPUNIT_ASSERT( privateTransport.getTransportProtocol() == "udp" );

      CPPUNIT_ASSERT( publicTransport.isEqual( privateTransport ) );
   }

   void testEndpointDescriptor_LocalNATedTcp()
   {
      NatTraversalRules natRules;
      UtlString     rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
      CPPUNIT_ASSERT( natRules.loadRules( rulesFile ) == OS_SUCCESS );

      // pick an IP address that is part of the local topology according to
      // the NatTraversalRules prescribed by /NatTraversalAgent/nattraversalrules1.xml
      Url localNATedContactUri( "<sip:601@10.10.10.10:6060;transport=tcp;x-sipX-nonat>" );

      EndpointDescriptor endpointDescriptor( localNATedContactUri, natRules );

      CPPUNIT_ASSERT( endpointDescriptor.getLocationCode() == LOCAL_NATED );

      TransportData publicTransport = endpointDescriptor.getPublicTransportAddress();
      CPPUNIT_ASSERT( publicTransport.getAddress() == "10.10.10.10" );
      CPPUNIT_ASSERT( publicTransport.getPort() == 6060 );
      CPPUNIT_ASSERT( publicTransport.getTransportProtocol() == "tcp" );

      TransportData privateTransport = endpointDescriptor.getNativeTransportAddress();
      CPPUNIT_ASSERT( privateTransport.getAddress() == "10.10.10.10" );
      CPPUNIT_ASSERT( privateTransport.getPort() == 6060 );
      CPPUNIT_ASSERT( privateTransport.getTransportProtocol() == "tcp" );

      CPPUNIT_ASSERT( publicTransport.isEqual( privateTransport ) );
   }

   void testEndpointDescriptor_UnknownUdpWithinLocalPrivateNetwork()
   {
      NatTraversalRules natRules;
      UtlString     rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
      CPPUNIT_ASSERT( natRules.loadRules( rulesFile ) == OS_SUCCESS );

      // pick an IP address that is part of the local topology according to
      // the NatTraversalRules prescribed by /NatTraversalAgent/nattraversalrules1.xml
      Url unknownContactUri( "<sip:601@10.10.10.11:6060>" );

      EndpointDescriptor endpointDescriptor( unknownContactUri, natRules );

      CPPUNIT_ASSERT( endpointDescriptor.getLocationCode() == LOCAL_NATED );

      TransportData publicTransport = endpointDescriptor.getPublicTransportAddress();
      CPPUNIT_ASSERT( publicTransport.getAddress() == UNKNOWN_IP_ADDRESS_STRING );
      CPPUNIT_ASSERT( publicTransport.getPort() == UNKNOWN_PORT_NUMBER );
      CPPUNIT_ASSERT( publicTransport.getTransportProtocol() == UNKNOWN_TRANSPORT_STRING );

      TransportData privateTransport = endpointDescriptor.getNativeTransportAddress();
      CPPUNIT_ASSERT( privateTransport.getAddress() == "10.10.10.11" );
      CPPUNIT_ASSERT( privateTransport.getPort() == 6060 );
      CPPUNIT_ASSERT( privateTransport.getTransportProtocol() == "udp" );

      CPPUNIT_ASSERT( !publicTransport.isEqual( privateTransport ) );
   }

   void testEndpointDescriptor_UnknownTcpWithinLocalPrivateNetwork()
   {
      NatTraversalRules natRules;
      UtlString     rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
      CPPUNIT_ASSERT( natRules.loadRules( rulesFile ) == OS_SUCCESS );

      // pick an IP address that is part of the local topology according to
      // the NatTraversalRules prescribed by /NatTraversalAgent/nattraversalrules1.xml
      Url unknownContactUri( "<sip:601@10.10.10.11:6060;transport=tcp>" );

      EndpointDescriptor endpointDescriptor( unknownContactUri, natRules );

      CPPUNIT_ASSERT( endpointDescriptor.getLocationCode() == LOCAL_NATED );

      TransportData publicTransport = endpointDescriptor.getPublicTransportAddress();
      CPPUNIT_ASSERT( publicTransport.getAddress() == UNKNOWN_IP_ADDRESS_STRING );
      CPPUNIT_ASSERT( publicTransport.getPort() == UNKNOWN_PORT_NUMBER );
      CPPUNIT_ASSERT( publicTransport.getTransportProtocol() == UNKNOWN_TRANSPORT_STRING );

      TransportData privateTransport = endpointDescriptor.getNativeTransportAddress();
      CPPUNIT_ASSERT( privateTransport.getAddress() == "10.10.10.11" );
      CPPUNIT_ASSERT( privateTransport.getPort() == 6060 );
      CPPUNIT_ASSERT( privateTransport.getTransportProtocol() == "tcp" );

      CPPUNIT_ASSERT( !publicTransport.isEqual( privateTransport ) );
   }

   void testEndpointDescriptor_UnknownUdpOutsideLocalPrivateNetwork()
   {
      NatTraversalRules natRules;
      UtlString     rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
      CPPUNIT_ASSERT( natRules.loadRules( rulesFile ) == OS_SUCCESS );

      // pick an IP address that is outside the local topology according to
      // the NatTraversalRules prescribed by /NatTraversalAgent/nattraversalrules1.xml
      Url unknownContactUri( "<sip:601@55.10.10.11:6060>" );

      EndpointDescriptor endpointDescriptor( unknownContactUri, natRules );

      CPPUNIT_ASSERT( endpointDescriptor.getLocationCode() == UNKNOWN );

      TransportData publicTransport = endpointDescriptor.getPublicTransportAddress();
      CPPUNIT_ASSERT( publicTransport.getAddress() == UNKNOWN_IP_ADDRESS_STRING );
      CPPUNIT_ASSERT( publicTransport.getPort() == UNKNOWN_PORT_NUMBER );
      CPPUNIT_ASSERT( publicTransport.getTransportProtocol() == UNKNOWN_TRANSPORT_STRING );

      TransportData privateTransport = endpointDescriptor.getNativeTransportAddress();
      CPPUNIT_ASSERT( privateTransport.getAddress() == "55.10.10.11" );
      CPPUNIT_ASSERT( privateTransport.getPort() == 6060 );
      CPPUNIT_ASSERT( privateTransport.getTransportProtocol() == "udp" );

      CPPUNIT_ASSERT( !publicTransport.isEqual( privateTransport ) );
   }

   void testEndpointDescriptor_UnknownTcpOutsideLocalPrivateNetwork()
   {
      NatTraversalRules natRules;
      UtlString     rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
      CPPUNIT_ASSERT( natRules.loadRules( rulesFile ) == OS_SUCCESS );

      // pick an IP address that is outside the local topology according to
      // the NatTraversalRules prescribed by /NatTraversalAgent/nattraversalrules1.xml
      Url unknownContactUri( "<sip:601@55.10.10.11:6060;transport=tcp>" );

      EndpointDescriptor endpointDescriptor( unknownContactUri, natRules );

      CPPUNIT_ASSERT( endpointDescriptor.getLocationCode() == UNKNOWN );

      TransportData publicTransport = endpointDescriptor.getPublicTransportAddress();
      CPPUNIT_ASSERT( publicTransport.getAddress() == UNKNOWN_IP_ADDRESS_STRING );
      CPPUNIT_ASSERT( publicTransport.getPort() == UNKNOWN_PORT_NUMBER );
      CPPUNIT_ASSERT( publicTransport.getTransportProtocol() == UNKNOWN_TRANSPORT_STRING );

      TransportData privateTransport = endpointDescriptor.getNativeTransportAddress();
      CPPUNIT_ASSERT( privateTransport.getAddress() == "55.10.10.11" );
      CPPUNIT_ASSERT( privateTransport.getPort() == 6060 );
      CPPUNIT_ASSERT( privateTransport.getTransportProtocol() == "tcp" );

      CPPUNIT_ASSERT( !publicTransport.isEqual( privateTransport ) );
   }

   void testMediaDescriptor_TypeAndDirectionality()
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
         "m=audio 49170 RTP/AVP 0\r\n"    // media description 0
         "c=IN IP4 224.2.17.12/127\r\n"
         "a=recvonly\r\n"
         "m=video 51372 RTP/AVP 31\r\n"   // media description 1
         "a=inactive\r\n"
         "m=application 32416 udp wb\r\n" // media description 2
         "a=sendonly\r\n"
         "m=audio 55554 RTP/AVP 0\r\n"    // media description 3
         "c=IN IP4 224.2.17.12/127\r\n"
         "a=sendrecv\r\n"
         "m=audio 55560 RTP/AVP 0\r\n"    // media description 4
         ;
      SdpBody body(sdp);

      MediaDescriptor md0( body, 0, CALLER );
      MediaDescriptor md1( body, 1, CALLER );
      MediaDescriptor md2( body, 2, CALLER );
      MediaDescriptor md3( body, 3, CALLER );
      MediaDescriptor md4( body, 4, CALLER );

      CPPUNIT_ASSERT( md0.getType() == "audio" );
      CPPUNIT_ASSERT( md0.getDirectionality() == RECV_ONLY );
      CPPUNIT_ASSERT( md0.getDirectionalityOverride() == NOT_A_DIRECTION );
      md0.setDirectionalityOverride( INACTIVE );
      CPPUNIT_ASSERT( md0.getDirectionalityOverride() == INACTIVE );

      CPPUNIT_ASSERT( md1.getType() == "video" );
      CPPUNIT_ASSERT( md1.getDirectionality() == INACTIVE );

      CPPUNIT_ASSERT( md2.getType() == "application" );
      CPPUNIT_ASSERT( md2.getDirectionality() == SEND_ONLY );

      CPPUNIT_ASSERT( md3.getType() == "audio" );
      CPPUNIT_ASSERT( md3.getDirectionality() == SEND_RECV );

      CPPUNIT_ASSERT( md4.getType() == "audio" );
      CPPUNIT_ASSERT( md4.getDirectionality() == SEND_RECV );
   }

   void testMediaDescriptor_GetEndpointData()
   {
      const char *sdpOffer =
         "v=0\r\n"
         "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
         "s=SDP Seminar\r\n"
         "i=A Seminar on the session description protocol\r\n"
         "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
         "e=mjh@isi.edu (Mark Handley)\r\n"
         "c=IN IP4 10.10.10.1\r\n"
         "t=2873397496 2873404696\r\n"
         "m=audio 10000 RTP/AVP 0\r\n"    // media description 0
         "c=IN IP4 10.10.10.2\r\n"
         "a=recvonly\r\n"
         "m=video 10002 RTP/AVP 31\r\n"   // media description 1
         "a=inactive\r\n"
         "m=application 10004 udp wb\r\n" // media description 2
         "a=sendonly\r\n"
         "m=audio 10006 RTP/AVP 0\r\n"    // media description 3
         "c=IN IP4 10.10.10.3\r\n"
         "a=sendrecv\r\n"
         "m=audio 10008 RTP/AVP 0\r\n"    // media description 4
         ;
      SdpBody offerBody(sdpOffer);

      const char *sdpAnswer =
         "v=0\r\n"
         "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
         "s=SDP Seminar\r\n"
         "i=A Seminar on the session description protocol\r\n"
         "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
         "e=mjh@isi.edu (Mark Handley)\r\n"
         "c=IN IP4 20.10.10.1\r\n"
         "t=2873397496 2873404696\r\n"
         "m=audio 20000 RTP/AVP 0\r\n"    // media description 0
         "c=IN IP4 20.10.10.2\r\n"
         "a=recvonly\r\n"
         "m=video 20002 RTP/AVP 31\r\n"   // media description 1
         "a=inactive\r\n"
         "m=application 20004 udp wb\r\n" // media description 2
         "a=sendonly\r\n"
         "m=audio 20006 RTP/AVP 0\r\n"    // media description 3
         "c=IN IP4 20.10.10.3\r\n"
         "a=sendrecv\r\n"
         "m=audio 20008 RTP/AVP 0\r\n"    // media description 4
         ;
      SdpBody answerBody(sdpAnswer);

      MediaDescriptor md0( offerBody,  0, CALLER );
      MediaDescriptor md1( offerBody,  1, CALLER );
      MediaDescriptor md2( offerBody,  2, CALLER );
      MediaDescriptor md3( offerBody,  3, CALLER );
      MediaDescriptor md4( offerBody,  4, CALLER );

      md0.setEndpointData( answerBody, 0, CALLEE );
      md1.setEndpointData( answerBody, 1, CALLEE );
      md2.setEndpointData( answerBody, 2, CALLEE );
      md3.setEndpointData( answerBody, 3, CALLEE );
      md4.setEndpointData( answerBody, 4, CALLEE );

      MediaEndpoint mediaEndpoint;

      // Media Descriptor 0
      mediaEndpoint = md0.getEndpoint( CALLER );
      CPPUNIT_ASSERT( mediaEndpoint.getAddress()  == "10.10.10.2" );
      CPPUNIT_ASSERT( mediaEndpoint.getRtpPort()  == 10000 );
      CPPUNIT_ASSERT( mediaEndpoint.getRtcpPort() == 10001 );

      mediaEndpoint = md0.getEndpoint( CALLEE );
      CPPUNIT_ASSERT( mediaEndpoint.getAddress()  == "20.10.10.2" );
      CPPUNIT_ASSERT( mediaEndpoint.getRtpPort()  == 20000 );
      CPPUNIT_ASSERT( mediaEndpoint.getRtcpPort() == 20001 );

      // Media Descriptor 1
      mediaEndpoint = md1.getEndpoint( CALLER );
      CPPUNIT_ASSERT( mediaEndpoint.getAddress()  == "10.10.10.1" );
      CPPUNIT_ASSERT( mediaEndpoint.getRtpPort()  == 10002 );
      CPPUNIT_ASSERT( mediaEndpoint.getRtcpPort() == 10003 );

      mediaEndpoint = md1.getEndpoint( CALLEE );
      CPPUNIT_ASSERT( mediaEndpoint.getAddress()  == "20.10.10.1" );
      CPPUNIT_ASSERT( mediaEndpoint.getRtpPort()  == 20002 );
      CPPUNIT_ASSERT( mediaEndpoint.getRtcpPort() == 20003 );

      // Media Descriptor 2
      mediaEndpoint = md2.getEndpoint( CALLER );
      CPPUNIT_ASSERT( mediaEndpoint.getAddress()  == "10.10.10.1" );
      CPPUNIT_ASSERT( mediaEndpoint.getRtpPort()  == 10004 );
      CPPUNIT_ASSERT( mediaEndpoint.getRtcpPort() == 10005 );

      mediaEndpoint = md2.getEndpoint( CALLEE );
      CPPUNIT_ASSERT( mediaEndpoint.getAddress()  == "20.10.10.1" );
      CPPUNIT_ASSERT( mediaEndpoint.getRtpPort()  == 20004 );
      CPPUNIT_ASSERT( mediaEndpoint.getRtcpPort() == 20005 );

      // Media Descriptor 3
      mediaEndpoint = md3.getEndpoint( CALLER );
      CPPUNIT_ASSERT( mediaEndpoint.getAddress()  == "10.10.10.3" );
      CPPUNIT_ASSERT( mediaEndpoint.getRtpPort()  == 10006 );
      CPPUNIT_ASSERT( mediaEndpoint.getRtcpPort() == 10007 );

      mediaEndpoint = md3.getEndpoint( CALLEE );
      CPPUNIT_ASSERT( mediaEndpoint.getAddress()  == "20.10.10.3" );
      CPPUNIT_ASSERT( mediaEndpoint.getRtpPort()  == 20006 );
      CPPUNIT_ASSERT( mediaEndpoint.getRtcpPort() == 20007 );

      // Media Descriptor 4
      mediaEndpoint = md4.getEndpoint( CALLER );
      CPPUNIT_ASSERT( mediaEndpoint.getAddress()  == "10.10.10.1" );
      CPPUNIT_ASSERT( mediaEndpoint.getRtpPort()  == 10008 );
      CPPUNIT_ASSERT( mediaEndpoint.getRtcpPort() == 10009 );

      mediaEndpoint = md4.getEndpoint( CALLEE );
      CPPUNIT_ASSERT( mediaEndpoint.getAddress()  == "20.10.10.1" );
      CPPUNIT_ASSERT( mediaEndpoint.getRtpPort()  == 20008 );
      CPPUNIT_ASSERT( mediaEndpoint.getRtcpPort() == 20009 );
   }

   void testMediaDescriptor_MediaRelayHandlesManip()
   {
      const char *sdpOffer =
         "v=0\r\n"
         "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
         "s=SDP Seminar\r\n"
         "c=IN IP4 10.10.10.1\r\n"
         "m=audio 10000 RTP/AVP 0\r\n"    // media description 0
         ;
      SdpBody offerBody(sdpOffer);

      MediaDescriptor md( offerBody,  0, CALLER );
      CPPUNIT_ASSERT( md.getCurrentMediaRelayHandle() == INVALID_MEDIA_RELAY_HANDLE );
      md.setCurrentMediaRelayHandle( 10 );
      CPPUNIT_ASSERT( md.getCurrentMediaRelayHandle() == 10 );
      md.clearCurrentMediaRelayHandle();
      CPPUNIT_ASSERT( md.getCurrentMediaRelayHandle() == INVALID_MEDIA_RELAY_HANDLE );

      CPPUNIT_ASSERT( md.getTentativeInitialMediaRelayHandle() == INVALID_MEDIA_RELAY_HANDLE );
      md.setTentativeInitialMediaRelayHandle( 20 );
      CPPUNIT_ASSERT( md.getTentativeInitialMediaRelayHandle() == 20 );
      md.clearTentativeInitialMediaRelayHandle();
      CPPUNIT_ASSERT( md.getTentativeInitialMediaRelayHandle() == INVALID_MEDIA_RELAY_HANDLE );

      CPPUNIT_ASSERT( md.getTentativeNonInitialMediaRelayHandle() == INVALID_MEDIA_RELAY_HANDLE );
      md.setTentativeNonInitialMediaRelayHandle( 30 );
      CPPUNIT_ASSERT( md.getTentativeNonInitialMediaRelayHandle() == 30 );
      md.clearTentativeNonInitialMediaRelayHandle();
      CPPUNIT_ASSERT( md.getTentativeNonInitialMediaRelayHandle() == INVALID_MEDIA_RELAY_HANDLE );
   }

   void testMediaDescriptor_DirectionalityConversions()
   {
      UtlString tmpDirectionalityString;
      MediaDescriptor::mediaDirectionalityValueToSdpDirectionalityAttribute( SEND_RECV, tmpDirectionalityString );
      CPPUNIT_ASSERT( tmpDirectionalityString == "sendrecv" );
      MediaDescriptor::mediaDirectionalityValueToSdpDirectionalityAttribute( SEND_ONLY, tmpDirectionalityString );
      CPPUNIT_ASSERT( tmpDirectionalityString == "sendonly" );
      MediaDescriptor::mediaDirectionalityValueToSdpDirectionalityAttribute( RECV_ONLY, tmpDirectionalityString );
      CPPUNIT_ASSERT( tmpDirectionalityString == "recvonly" );
      MediaDescriptor::mediaDirectionalityValueToSdpDirectionalityAttribute( INACTIVE, tmpDirectionalityString );
      CPPUNIT_ASSERT( tmpDirectionalityString == "inactive" );
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(NatTraversalDataTypesTest);
